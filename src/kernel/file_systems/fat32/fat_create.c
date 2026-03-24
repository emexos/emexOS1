#include "fat32_internal.h"
#include <memory/main.h>
#include <string/string.h>
#include <kernel/communication/serial.h>

static u8 cbuf[4096] __attribute__((aligned(16)));

static u32 cs(void) {
    return bootSector.sectors_per_cluster * bootSector.bytes_per_sector;
}
static void read_cluster(uint32_t c) {
    u32 lba = cluster_to_Lba(c);
    for (u32 s = 0; s < bootSector.sectors_per_cluster; s++)
        ATAread_sectors(disk_device, lba + s, 1,
            (u16*)(cbuf + s * bootSector.bytes_per_sector));
}
static void write_cluster(uint32_t c) {
    u32 lba = cluster_to_Lba(c);
    for (u32 s = 0; s < bootSector.sectors_per_cluster; s++)
        ATAwrite_sectors(disk_device, lba + s, 1,
            (u16*)(cbuf + s * bootSector.bytes_per_sector));
}
static void make_sfn(const char *name, char sfn[11])
{
    char tmp[12];
    int has_dot = 0;
    memset(tmp, 0, 12);
    string_to_fatname(name, tmp);

    //names withouth ext == de stores as 0x00 (NOT 0x20)
    // in ext_bytes 8-10

    // force write 0
    for (
    	const char *p = name; *p; p++
    )
    if (*p == '.') {
    	has_dot = 1; break;
    }


    if (!has_dot) { tmp[8] = tmp[9] = tmp[10] = 0; }

    memcpy(sfn, tmp, 11);
}
static u8 lfn_sum(const char sfn[11]) {
    u8 s = 0;
    for (int i = 11; i > 0; i--)
        s = ((s & 1) ? 0x80 : 0) + (s >> 1) + (u8)*sfn++;
    return s;
}
static uint32_t dir_lookup(uint32_t dir_cluster, const char *name)
{
    char fn[11];
    memset(fn, 0, 11);
    make_sfn(name, fn);
    uint32_t epc = cs() / 32;
    uint32_t cur = dir_cluster;
    while (cur < 0x0FFFFFF8) {
        read_cluster(cur);
        fat_dir_entry_t *e = (fat_dir_entry_t*)cbuf;
        for (uint32_t i = 0; i < epc; i++) {
            if ((u8)e[i].filename[0] == 0x00) return 0;
            if ((u8)e[i].filename[0] == 0xE5) continue;
            if (e[i].attributes == 0x0F) continue;
            if (memcmp(e[i].filename, fn, 11) == 0)
                return e[i].firstClusterLow | ((uint32_t)e[i].firstClusterHigh << 16);
        }
        cur = FAT[cur] & 0x0FFFFFFF;
    }
    return 0;
}

// walk path, write last component to out_name, return parent dir cluster
static uint32_t split_path(const char *path, char *out_name) {
    int last = -1;
    for (int i = 0; path[i]; i++)
        if (path[i] == '/') last = i;

    str_copy(out_name, path + last + 1);
    if (last <= 0) return bootSector.root_cluster;

    uint32_t cur = bootSector.root_cluster;
    int pos = 1;
    while (pos < last) {
        char comp[64];
        int ci = 0;
        while (pos < last && path[pos] != '/') {
            if (ci < 63) comp[ci++] = path[pos];
            pos++;
        }
        comp[ci] = '\0';
        while (pos < last && path[pos] == '/') pos++;
        if (!comp[0]) continue;
        uint32_t next = dir_lookup(cur, comp);
        if (!next) return 0;
        cur = next;
    }
    return cur;
}

static int find_slots(uint32_t dir_cluster, int n, uint32_t *out_c, int *out_i) {
    uint32_t epc = cs() / 32;
    uint32_t cur = dir_cluster;
    while (1) {
        read_cluster(cur);
        fat_dir_entry_t *e = (fat_dir_entry_t*)cbuf;
        for (int i = 0; i <= (int)epc - n; i++) {
            int ok = 1;
            for (int j = 0; j < n && ok; j++) {
                u8 f = (u8)e[i+j].filename[0];
                if (f != 0x00 && f != 0xE5) ok = 0;
            }
            if (ok) { *out_c = cur; *out_i = i; return 0; }
        }
        uint32_t next = FAT[cur] & 0x0FFFFFFF;
        if (next >= 0x0FFFFFF8) {
            uint32_t nc = alloc_cluster();
            if (!nc) return -1;
            FAT[cur] = nc;
            FAT[nc]  = 0x0FFFFFFF;
            memset(cbuf, 0, cs());
            write_cluster(nc);
            cur = nc;
        } else {
            cur = next;
        }
    }
}

static int write_dir_entry(
	uint32_t dir_cluster, const char *name,
	uint32_t first_cluster, uint32_t size, u8 attr
){
    int namelen = str_len(name);
    int lfn_count = (namelen + 12) / 13;
    int total = lfn_count + 1;

    char sfn[11];
    make_sfn(name, sfn);
    u8 csum = lfn_sum(sfn);

    uint32_t slot_c; int slot_i;
    if (find_slots(dir_cluster, total, &slot_c, &slot_i) != 0) return -1;

    read_cluster(slot_c);

    u16 uname[128];
    for (int i = 0; i < namelen; i++) uname[i] = (u16)(u8)name[i];
    if (namelen < (int)(sizeof(uname)/2)) uname[namelen] = 0x0000;
    for (int i = namelen + 1; i < lfn_count * 13; i++) uname[i] = 0xFFFF;

    typedef struct {
        u8 order;
        u16 name1[5];
        u8 attr;
        u8 type;
        u8 checksum;
        u16 name2[6];
        u16 first_cluster;
        u16 name3[2];
    } __attribute__((packed)) lfn_t;
    lfn_t *le = (lfn_t*)cbuf;

    for (int k = 0; k < lfn_count; k++) {
        int chunk = lfn_count - 1 - k;
        lfn_t *l  = &le[slot_i + k];
        memset(l, 0, sizeof(lfn_t));

        l->order = (u8)(chunk + 1);
        if (k == 0) l->order |= 0x40;

        l->attr = 0x0F;
        l->checksum = csum;
        l->first_cluster = 0;

        u16 *src = uname + chunk * 13;

        memcpy(l->name1, src,    10);
        memcpy(l->name2, src + 5,12);
        memcpy(l->name3, src + 11,4);
    }

    fat_dir_entry_t *de = (fat_dir_entry_t*)&le[slot_i + lfn_count];
    memset(de, 0, sizeof(fat_dir_entry_t));
    memcpy(de->filename, sfn, 11);
    de->attributes = attr;
    de->firstClusterLow = first_cluster & 0xFFFF;
    de->firstClusterHigh = (first_cluster >> 16) & 0xFFFF;
    de->fileSize = size;

    write_cluster(slot_c);

    return 0;
}

int fat32_create_dir(const char *path)
{
    if (!fat32_initialized) return -1;

    char name[64];
    uint32_t parent = split_path(path, name);
    if (!parent || !name[0]) return -1;

    if (dir_lookup(parent, name)) return 0; // already exists

    uint32_t nc = alloc_cluster();
    if (!nc) return -1;
    FAT[nc] = 0x0FFFFFFF;
    memset(cbuf, 0, cs());
    write_cluster(nc);

    if (write_dir_entry(parent, name, nc, 0, 0x10) != 0) return -1;

    flush_fat();
    return 0;
}

int fat32_write_file(const char *path, const void *data, u32 size)
{
    char name[64];
    uint32_t parent = split_path(path, name);
    u32 csize = cs();
    uint32_t first = 0, prev = 0;
    u32 done = 0;

    if (!fat32_initialized) return -1;
    if (!parent || !name[0]) return -1;
    if (size == 0) {
        write_dir_entry(parent, name, 0, 0, 0x20);
        flush_fat();
        return 0;
    }

    while (done < size)
    {
        uint32_t c = alloc_cluster();
        if (!c) return -1;
        FAT[c] = 0x0FFFFFFF;
        if (prev) FAT[prev] = c;
        if (!first) first = c;

        u32 to_copy = size - done;
        if (to_copy > csize) to_copy = csize;

        memset(cbuf, 0, csize);
        memcpy(cbuf, (u8*)data + done, to_copy);
        write_cluster(c);

        prev = c;
        done += to_copy;
    }

    if (write_dir_entry(parent, name, first, size, 0x20) != 0) return -1;

    flush_fat();

    return (int)done;
}
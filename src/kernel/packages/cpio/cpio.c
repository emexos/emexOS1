#include "cpio.h"

#include <kernel/file_systems/vfs/vfs.h>
#include <string/string.h>
#include <memory/main.h>
#include <kernel/communication/serial.h>
#include <kernel/graph/theme.h>
#include <theme/doccr.h>

static u32 parse_hex8(const char **p)
{
    u32 val = 0;
    for (int i = 0; i < 8; i++) {
        char c = (*p)[i];
        u32 n;
        if (c >= '0' && c <= '9') n = (u32)(c - '0');
        else if (c >= 'a' && c <= 'f') n = (u32)(c - 'a' + 10);
        else if (c >= 'A' && c <= 'F') n = (u32)(c - 'A' + 10);
        else n = 0;
        val = (val << 4) | n;
    }
    *p += 8;
    return val;
}

static inline u64 align4(u64 n){ return (n + 3) & ~(u64)3; }

// pub iterators
//
void cpio_iter_init(cpio_iter_t *iter, const u8 *data, u64 size)
{
    if (!iter) return;
    iter->base = data;
    iter->size = size;
    iter->offset = 0;
}

int cpio_iter_next(cpio_iter_t *iter, cpio_entry_t *entry)
{
    if (!iter || !entry) return CPIO_ERR_TRUNC;

    u64 off = iter->offset;

    if (off+CPIO_HEADER_SIZE > iter->size) return CPIO_ERR_TRUNC;

    const char *hdr = (const char *)(iter->base + off);

    if (!(hdr[0]=='0' && hdr[1]=='7' && hdr[2]=='0' &&
          hdr[3]=='7' && hdr[4]=='0' && (hdr[5]=='1' || hdr[5]=='2')))
    return CPIO_ERR_MAGIC;

    const char *p = hdr + 6;
    entry->ino = parse_hex8(&p);
    entry->mode = parse_hex8(&p);
    entry->uid = parse_hex8(&p);
    entry->gid = parse_hex8(&p);
    entry->nlink = parse_hex8(&p);
    entry->mtime = parse_hex8(&p);
    entry->filesize = parse_hex8(&p);
    entry->devmajor = parse_hex8(&p);
    entry->devminor = parse_hex8(&p);
    entry->rdevmajor = parse_hex8(&p);
    entry->rdevminor = parse_hex8(&p);
    entry->namesize = parse_hex8(&p);
    parse_hex8(&p); // crc field / ignored

    off += CPIO_HEADER_SIZE;

    if (off+ entry->namesize > iter->size) return CPIO_ERR_TRUNC;
    entry->name = (const char *)(iter->base + off);

    off = iter->offset + align4(CPIO_HEADER_SIZE + entry->namesize);

    if (str_equals(entry->name, CPIO_TRAILER)){
        iter->offset = off;

        return CPIO_ERR_EOF;
    }

    if (off + entry->filesize > iter->size) return CPIO_ERR_TRUNC;
    entry->data = iter->base + off;
    iter->offset = off + align4(entry->filesize);

    return CPIO_OK;
}

#define CPIO_MAX_ENTRIES 512

typedef struct {
    cpio_entry_t e;
    char path[256];
} cpio_staged_t;

static cpio_staged_t _staged[CPIO_MAX_ENTRIES];

static void make_path(char *out, const char *base, const char *name)
{
    // strip leading "./" or lone "."
    if (name[0] == '.' && name[1] == '/') name += 2;
    if (name[0] == '.' && name[1] == '\0') { str_copy(out, base); return; }

    if (str_equals(base, "/")) {
        out[0] = '/';
        str_copy(out + 1, name);
    } else {
        str_copy(out, base);
        int bl = str_len(out);
        if (bl > 0 && out[bl-1] != '/') str_append(out, "/");
        str_append(out, name);
    }
    // remove trailing slash unless root
    int l = str_len(out);
    if (l >1 && out[l-1] == '/') out[l-1] = '\0';
}

int cpio_extract_to_vfs(const u8 *data, u64 size, const char *base_path)
{
    if (!data || size == 0 || !base_path) return CPIO_ERR_TRUNC;

    cpio_iter_t  iter;
    cpio_entry_t entry;
    cpio_iter_init(&iter, data, size);

    int total = 0;
    int staged = 0;

    // self explaining i think... reading archive....
    print("\n", white());
    log("[CPIO]", "reading archive...\n", d);

    while (staged < CPIO_MAX_ENTRIES) {
        int rc = cpio_iter_next(&iter, &entry);
        if (rc == CPIO_ERR_EOF) break;
        if (rc != CPIO_OK) { log("[CPIO]", "parse error\n", error); return rc; }

        const char *n = entry.name;
        //strips "./" or "."
        if (n[0]=='.' && n[1]=='\0') continue;
        if (n[0]=='.' && n[1]=='/' && n[2]=='\0') continue;
        if (!cpio_is_file(&entry) && !cpio_is_dir(&entry) && !cpio_is_symlink(&entry)) continue;

        cpio_staged_t *s = &_staged[staged];
        s->e = entry;
        make_path(s->path, base_path, entry.name);

        if (cpio_is_dir(&entry)) {
            log("[CPIO]"," dir  ", d);}
        else {
            log("[CPIO]"," file ", d);}
        print(s->path, white());
        if (cpio_is_file(&entry) && entry.filesize > 0) {
            char buf[20];
            str_copy(buf, " (");
            str_append_uint(buf, entry.filesize); // shows how many bytes a file has
            str_append(buf, "B)");
            print(buf, white());
        }
        print("\n", white());
        staged++;
    }
    print("\n", white());

    // only now create all directorys
    // (just made it so it looks better...)
    for (int i = 0; i < staged; i++) {
        if (cpio_is_dir(&_staged[i].e)) {
            fs_mkdir(_staged[i].path);
            total++;
        }
    }

    // this writes the files but it can also overwrite files
    for (int i = 0; i < staged; i++)
    {
        cpio_staged_t *s = &_staged[i];
        if (!cpio_is_file(&s->e) && !cpio_is_symlink(&s->e)) continue;

        int fd = fs_open(s->path, O_CREAT | O_WRONLY);
        if (fd < 0) continue;
        if (s->e.filesize > 0)
            fs_write(fd, (void *)s->e.data, s->e.filesize);
        fs_close(fd);
        total++;
    }

    char buf[32];
    str_copy(buf, "");
    str_append_uint(buf, (u32)total);
    log("[CPIO]", "extracted ", d);
    print(buf, white());
    print(" entries\n", white());

    return total;
}

int cpio_find(const u8 *data, u64 size, const char *name, cpio_entry_t *entry)
{
    if (!data || !name || !entry) return CPIO_ERR_TRUNC;

    cpio_iter_t iter;
    cpio_iter_init(&iter, data, size);

    while (1) {
        int rc = cpio_iter_next(&iter, entry);
        if (rc == CPIO_ERR_EOF) return CPIO_ERR_EOF;
        if (rc != CPIO_OK)      return rc;

        const char *n = entry->name;
        if (n[0]=='.' && n[1]=='/') n += 2;
        if (str_equals(n, name)) return CPIO_OK;
    }
}

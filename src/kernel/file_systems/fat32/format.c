// format.c
#include "fat32.h"
#include <drivers/storage/ata/disk.h>
#include <memory/main.h>
#include <kernel/mem/klime/klime.h>
#include <kernel/communication/serial.h>
#include <theme/stdclrs.h>
#include <kernel/graph/theme.h>
#include <theme/doccr.h>

extern void *fs_klime;

typedef struct {
    u8  jmp_boot[3];
    u8  oem_name[8];
    u16 bytes_per_sector;
    u8  sectors_per_cluster;
    u16 reserved_sectors;
    u8  num_fats;
    u16 root_entries;
    u16 total_sectors_16;
    u8  media_type;
    u16 fat_size_16;
    u16 sectors_per_track;
    u16 num_heads;
    u32 hidden_sectors;
    u32 total_sectors_32;

    // FAT32 specific
    u32 fat_size_32;
    u16 ext_flags;
    u16 fs_version;
    u32 root_cluster;
    u16 fs_info;
    u16 backup_boot_sector;
    u8  reserved[12];
    u8  drive_number;
    u8  reserved1;
    u8  boot_signature;
    u32 volume_id;
    u8  volume_label[11];
    u8  fs_type[8];
    u8  boot_code[420];
    u16 signature;
} __attribute__((packed)) fat32_boot_sector_t;

int fat32_format_partition(u32 start_lba, u32 sector_count) {
    BOOTUP_PRINT("[FAT32] ", GFX_GRAY_70);
    BOOTUP_PRINT("Formatting partition at LBA ", white());
    BOOTUP_PRINT_INT(start_lba, white());
    BOOTUP_PRINT(" with ", white());
    BOOTUP_PRINT_INT(sector_count, white());
    BOOTUP_PRINT(" sectors...\n", white());

    ATAdevice_t *device = ATAget_device(0);
    u8 *boot_sector_buffer = (u8*)klime_create((klime_t*)fs_klime, 512);

    if (!device) {
        BOOTUP_PRINT("[FAT32] ", GFX_GRAY_70);
        BOOTUP_PRINT("No device\n", red());
        return -1;
    }
    if (!boot_sector_buffer) {
        BOOTUP_PRINT("[FAT32] ", GFX_GRAY_70);
        BOOTUP_PRINT("Failed to allocate boot sector buffer\n", red());
        return -1;
    }

    memset(boot_sector_buffer, 0, 512);
    fat32_boot_sector_t *bs = (fat32_boot_sector_t*)boot_sector_buffer;

    // jump ins
    bs->jmp_boot[0] = 0xEB;
    bs->jmp_boot[1] = 0x58;
    bs->jmp_boot[2] = 0x90;

    memcpy(bs->oem_name, "EMEXOS  ", 8); //OEM should be 8 chars

    // BPB
    bs->bytes_per_sector = 512;
    bs->sectors_per_cluster = 8; // 4KB clusters
    bs->reserved_sectors = 32;
    bs->num_fats = 2;
    bs->root_entries = 0; // FAT32 uses clusters for root
    bs->total_sectors_16 = 0;
    bs->media_type = 0xF8;
    bs->fat_size_16 = 0;
    bs->sectors_per_track = 63;
    bs->num_heads = 255;
    bs->hidden_sectors = 0;  // should be 0 for partition-relative
    bs->total_sectors_32 = sector_count;

    // FAT size
    u32 tmp1 = sector_count - bs->reserved_sectors;
    u32 tmp2 = (256 * bs->sectors_per_cluster) + bs->num_fats;
    u32 fat_size = (tmp1 + (tmp2 - 1)) / tmp2; //size

    bs->fat_size_32 = fat_size;
    bs->ext_flags = 0;
    bs->fs_version = 0;
    bs->root_cluster = 2;
    bs->fs_info = 1;
    bs->backup_boot_sector = 6;

    bs->drive_number = 0x80;
    bs->boot_signature = 0x29;
    bs->volume_id = 0x12345678;
    memcpy(bs->volume_label, "EMEXOS     ", 11);
    memcpy(bs->fs_type, "FAT32   ", 8);

    // signatur
    bs->signature = 0xAA55;

    BOOTUP_PRINT("[FAT32] ", GFX_GRAY_70);
    BOOTUP_PRINT("size: ", white());
    BOOTUP_PRINT_INT(fat_size, white());
    BOOTUP_PRINT(" sectors\n", white());
    BOOTUP_PRINT("[FAT32] ", GFX_GRAY_70);
    BOOTUP_PRINT("boot sector to LBA ", white());
    BOOTUP_PRINT_INT(start_lba, white());
    BOOTUP_PRINT("...\n", white());

    if (ATAwrite_sectors(device, start_lba, 1, (u16*)boot_sector_buffer) != 0) {
        BOOTUP_PRINT("[FAT32] ", GFX_GRAY_70);
        BOOTUP_PRINT("failed to write boot esctor\n", red());
        klime_free((klime_t*)fs_klime, (u64*)boot_sector_buffer);
        return -1;
    }

    // when boot sector was written
    u8 *verify_buffer = (u8*)klime_create((klime_t*)fs_klime, 512);
    if (verify_buffer) {
        if (ATAread_sectors(device, start_lba, 1, (u16*)verify_buffer) == 0) {
            BOOTUP_PRINT("[FAT32] ", GFX_GRAY_70);
            BOOTUP_PRINT("boot sig: 0x", white()); //hex
            BOOTUP_PRINT_INT(verify_buffer[510], white());
            BOOTUP_PRINT_INT(verify_buffer[511], white());
            BOOTUP_PRINT(" ; type: '", white());
            for (int i = 0; i < 8; i++) {
                char c = verify_buffer[82 + i];
                if (c >= 32 && c < 127) {
                    char buf[2] = {c, 0};
                    BOOTUP_PRINT(buf, white());
                }
            }
            BOOTUP_PRINT("'\n", white());
        }
        klime_free((klime_t*)fs_klime, (u64*)verify_buffer);
    }

    // backup
    BOOTUP_PRINT("[FAT32] ", GFX_GRAY_70);
    BOOTUP_PRINT("write backup boot sector\n", white());
    ATAwrite_sectors(device, start_lba + 6, 1, (u16*)boot_sector_buffer);

    // info secotr
    u8 *fsinfo = (u8*)klime_create((klime_t*)fs_klime, 512);
    if (fsinfo) {
        memset(fsinfo, 0, 512);

        // FSInfo signatures
        fsinfo[0]   = 'R'; fsinfo[1]   = 'R'; fsinfo[2]   = 'a'; fsinfo[3]   = 'A';  // lead
        fsinfo[484] = 'r'; fsinfo[485] = 'r'; fsinfo[486] = 'A'; fsinfo[487] = 'a';  // struct



        *(u32*)(fsinfo + 488) = 0xFFFFFFFF;
        *(u32*)(fsinfo + 492) = 3;
        // trail
        fsinfo[510] = 0x55;
        fsinfo[511] = 0xAA;

        ATAwrite_sectors(device, start_lba + 1, 1, (u16*)fsinfo);
        klime_free((klime_t*)fs_klime, (u64*)fsinfo);
    }

    //tables
    u8 *fat_sector = (u8*)klime_create((klime_t*)fs_klime, 512);
    if (!fat_sector) { // f
        BOOTUP_PRINT("[FAT32] ", GFX_GRAY_70);
        BOOTUP_PRINT("failed: allocate FAT buffer\n", red());
        klime_free((klime_t*)fs_klime, (u64*)boot_sector_buffer);
        return -1;
    }

    memset(fat_sector, 0, 512);
    u32 *fat_buffer = (u32*)fat_sector;

    // entries
    fat_buffer[0] = 0x0FFFFFF8;// media type
    fat_buffer[1] = 0x0FFFFFFF;// EOC
    fat_buffer[2] = 0x0FFFFFFF;// root cluster (EOC)

    BOOTUP_PRINT("[FAT32] ", GFX_GRAY_70);
    BOOTUP_PRINT("writes tables\n", white());

    // Write first fat1
    u32 fat_start = start_lba + bs->reserved_sectors;
    if (ATAwrite_sectors(device, fat_start, 1, (u16*)fat_sector) != 0) {
        BOOTUP_PRINT("[FAT32] ", GFX_GRAY_70);
        BOOTUP_PRINT("failed: write FAT1 \n", red());
        klime_free((klime_t*)fs_klime, (u64*)fat_sector);
        klime_free((klime_t*)fs_klime, (u64*)boot_sector_buffer);
        return -1;
    }

    // clear
    memset(fat_sector, 0, 512);
    for (u32 i = 1; i < fat_size; i++) {
        ATAwrite_sectors(device, fat_start + i, 1, (u16*)fat_sector);
    }

    // fat2
    BOOTUP_PRINT("[FAT32] ", GFX_GRAY_70);
    BOOTUP_PRINT("Copying to FAT2...\n", white());

    // restore again
    fat_buffer[0] = 0x0FFFFFF8;
    fat_buffer[1] = 0x0FFFFFFF;
    fat_buffer[2] = 0x0FFFFFFF;

    if (ATAwrite_sectors(device, fat_start + fat_size, 1, (u16*)fat_sector) != 0) {
        BOOTUP_PRINT("[FAT32] ", GFX_GRAY_70);
        BOOTUP_PRINT("failed: write FAT2\n", red());
        klime_free((klime_t*)fs_klime, (u64*)fat_sector);
        klime_free((klime_t*)fs_klime, (u64*)boot_sector_buffer);
        return -1;
    }

    // ckear
    memset(fat_sector, 0, 512);
    for (u32 i = 1; i < fat_size; i++) {
        ATAwrite_sectors(device, fat_start + fat_size + i, 1, (u16*)fat_sector);
    }

    // clear root
    memset(fat_sector, 0, 512);
    u32 root_lba = start_lba + bs->reserved_sectors + (bs->num_fats * fat_size);

    for (u32 i = 0; i < bs->sectors_per_cluster; i++) {
        ATAwrite_sectors(device, root_lba + i, 1, (u16*)fat_sector);
    }

    klime_free((klime_t*)fs_klime, (u64*)fat_sector);
    klime_free((klime_t*)fs_klime, (u64*)boot_sector_buffer);

    BOOTUP_PRINT("[FAT32] ", GFX_GRAY_70);
    BOOTUP_PRINT("format finish\n", green());

    return 0;
}

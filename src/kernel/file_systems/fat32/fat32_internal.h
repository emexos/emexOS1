#pragma once

#include <types.h>
#include "fat32.h"
#include <drivers/storage/ata/disk.h>

typedef struct {
    u8 jmp_boot[3];
    u8 oem_name[8];
    u16 bytes_per_sector;
    u8 sectors_per_cluster;
    u16 reserved_sectors;
    u8 num_fats;
    u16 root_entries;
    u16 total_sectors_16;
    u8 media_type;
    u16 fat_size_16;
    u16 sectors_per_track;
    u16 num_heads;
    u32 hidden_sectors;
    u32 total_sectors_32;
    u32 fat_size_32;
    u16 ext_flags;
    u16 fs_version;
    u32 root_cluster;
    u16 fs_info;
    u16 backup_boot_sector;
    u8 reserved[12];
    u8 drive_number;
    u8 reserved1;
    u8 boot_signature;
    u32 volume_id;
    u8 volume_label[11];
    u8 fs_type[8];
} __attribute__((packed)) fat_BS_t;

extern fat_BS_t bootSector;
extern uint32_t *FAT;
extern ATAdevice_t *disk_device;
extern int fat32_initialized;
extern u32 partition_start_lba;

extern uint32_t cluster_to_Lba(uint32_t cluster);
extern void string_to_fatname(const char *name, char *nameOut);

// fat32 write
extern uint32_t alloc_cluster(void);
extern void flush_fat(void);
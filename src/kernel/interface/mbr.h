// src/kernel/interface/mbr.h
#ifndef MBR_H
#define MBR_H

#include <types.h>

#define MBR_PARTITION_EMPTY     0x00
#define MBR_PARTITION_FAT12     0x01
#define MBR_PARTITION_FAT16     0x04
#define MBR_PARTITION_EXTENDED  0x05
#define MBR_PARTITION_FAT16B    0x06
#define MBR_PARTITION_NTFS      0x07
#define MBR_PARTITION_FAT32     0x0B
#define MBR_PARTITION_FAT32_LBA 0x0C
#define MBR_PARTITION_FAT16_LBA 0x0E
#define MBR_PARTITION_EXTENDED_LBA 0x0F
#define MBR_PARTITION_LINUX     0x83
#define MBR_PARTITION_LINUX_SWAP 0x82
#define MBR_PARTITION_EFI       0xEF

#define MBR_SIGNATURE 0xAA55

typedef struct {
    u8  status;
    u8  first_chs[3];
    u8  partition_type;
    u8  last_chs[3];
    u32 first_lba;
    u32 sector_count;
} __attribute__((packed)) mbr_partition_t;

typedef struct {
    u8  bootcode[446];
    mbr_partition_t partitions[4];
    u16 signature;
} __attribute__((packed)) mbr_t;

int mbr_read(u8 *buffer);
int mbr_write(const mbr_t *mbr);
int mbr_is_valid(const mbr_t *mbr);
void mbr_create_empty(mbr_t *mbr);
int mbr_create_partition(mbr_t *mbr, u8 index, u8 type, u32 start_lba, u32 sector_count);
const char* mbr_get_partition_type_name(u8 type);

#endif

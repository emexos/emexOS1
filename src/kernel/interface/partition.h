#ifndef PARTITION_H
#define PARTITION_H

#include <types.h>

#define MAX_PARTITIONS 4

#define PARTSECSIZE  65536

typedef struct {
    u8  valid;
    u8  bootable;
    u8  type;
    u32 start_lba;
    u32 sector_count;
    char type_name[32];
} partition_info_t;

int partition_init(void);
int partition_get_count(void);
partition_info_t* partition_get_info(int index);
int partition_format_disk_fat32(void);
int partition_needs_format(void);

#endif

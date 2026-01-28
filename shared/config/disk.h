// shared/config/disk.h
#pragma once

// Auto-format disk if not formatted
// WARNING: This will ERASE ALL DATA on the disk!
// Set to 1 to enable automatic formatting
// Set to 0 to require manual formatting
#define OVERWRITEALL 0

// Partition configuration
#define PARTITION_START_LBA 2048
#define PARTITION_TYPE_FAT32 0x0C  // FAT32 LBA

// FAT32 filesystem configuration
#define FAT32_CLUSTER_SIZE 8       // 8 sectors = 4KB clusters
#define FAT32_RESERVED_SECTORS 32
#define FAT32_NUM_FATS 2
#define FAT32_ROOT_CLUSTER 2

#define FAT32_VOLUME_LABEL "EMEXOS"
#define FAT32_OEM_NAME "EMEX4.0"

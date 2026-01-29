#pragma once

// WARNING: This will ERASE ALL DATA on the disk!
// 1 == enable automatic formatting
// 0 == require manual formatting
#define OVERWRITEALL 1


#define PARTITION_START_LBA 2048
#define PARTITION_TYPE_FAT32 0x0C  // FAT32 LBA

#define FAT32_CLUSTER_SIZE 8       // 4KB clusters
#define FAT32_RESERVED_SECTORS 32
#define FAT32_NUM_FATS 2
#define FAT32_ROOT_CLUSTER 2

#define FAT32_VOLUME_LABEL "EMEXOS"
#define FAT32_OEM_NAME "EMEX0.5"

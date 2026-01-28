#ifndef FAT32_H
#define FAT32_H


// from https://github.com/Novice06/fat32
//
//
#include <types.h>

typedef struct fat_dir_entry
{
    char filename[11];           // File name (8 characters) and File extension (3 characters)
    uint8_t attributes;          // File attributes (read-only, hidden, system, etc.)
    uint8_t reserved;            // Reserved (usually zero)
    uint8_t creationTimeTenth;   // Creation time in tenths of a second
    uint16_t creationTime;       // Creation time (HHMMSS format)
    uint16_t creationDate;       // Creation date (YYMMDD format)
    uint16_t lastAccessDate;     // Last access date
    uint16_t firstClusterHigh;   // High word of the first cluster number (FAT32 only)
    uint16_t writeTime;          // Last write time
    uint16_t writeDate;          // Last write date
    uint16_t firstClusterLow;    // Low word of the first cluster number
    uint32_t fileSize;           // File size in bytes
} __attribute__((packed)) fat_dir_entry_t;

typedef struct file
{
    fat_dir_entry_t* entry;
    uint32_t readPos;
} file_t;

void fat32_init(void);
int fat32_format_partition(u32 start_lba, u32 sector_count);
file_t* fat32_open(const char* path);
int fat32_read(file_t* this_file, void* buffer, size_t size);
void fat32_close(file_t* file);

#endif

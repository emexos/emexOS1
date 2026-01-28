// src/kernel/file_systems/fat32/fat32.c
#include "fat32.h"
#include <drivers/storage/ata/disk.h>
#include <memory/main.h>
#include <string/string.h>
#include <kernel/mem/klime/klime.h>
#include <kernel/communication/serial.h>
#include <theme/stdclrs.h>
#include <kernel/graph/theme.h>
#include <theme/doccr.h>
#include <kernel/interface/partition.h>

extern void *fs_klime;

typedef struct fat_BS
{
    uint8_t bootjmp[3];
    uint8_t oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sector_count;
    uint8_t table_count;
    uint16_t root_entry_count;
    uint16_t total_sectors_16;
    uint8_t media_type;
    uint16_t table_size_16;
    uint16_t sectors_per_track;
    uint16_t head_side_count;
    uint32_t hidden_sector_count;
    uint32_t total_sectors_32;

    //extended fat32 stuff
    uint32_t table_size_32;
    uint16_t extended_flags;
    uint16_t fat_version;
    uint32_t root_cluster;
    uint16_t fat_info;
    uint16_t backup_BS_sector;
    uint8_t reserved_0[12];
    uint8_t drive_number;
    uint8_t reserved_1;
    uint8_t boot_signature;
    uint32_t volume_id;
    uint8_t volume_label[11];
    uint8_t fat_type_label[8];

    uint8_t Filler[422];

}__attribute__((packed)) fat_BS_t;

typedef enum
{
    FAT_ATTR_REGULAR    = 0x00,
    FAT_ATTR_READ_ONLY  = 0x01,
    FAT_ATTR_HIDDEN     = 0x02,
    FAT_ATTR_SYSTEM     = 0x04,
    FAT_ATTR_VOLUME_ID  = 0x08,
    FAT_ATTR_DIRECTORY  = 0x10,
    FAT_ATTR_ARCHIVE    = 0x20,
    FAT_ATTR_LFN        = 0x0F
} fat_attributes_t;

typedef struct {
    uint8_t  order;
    uint16_t name1[5];
    uint8_t  attr;
    uint8_t  type;
    uint8_t  checksum;
    uint16_t name2[6];
    uint16_t firstCluster;
    uint16_t name3[2];
}__attribute__((packed)) LFN;

fat_BS_t bootSector;
uint32_t* FAT;
void* working_buffer;
static ATAdevice_t *disk_device;
static int fat32_initialized = 0;
static u32 partition_start_lba = 0;

void fat32_init(void)
{
    printf("[FAT32] Initializing FAT32 driver\n");

    // Get first ATA device
    disk_device = ATAget_device(0);
    if (!disk_device) {
        BOOTUP_PRINT("[FAT32] ", GFX_GRAY_70);
        BOOTUP_PRINT("No ATA device found\n", red());
        return;
    }

    // find FAT32 partition
    partition_info_t *fat32_part = NULL;
    int part_count = partition_get_count();

    BOOTUP_PRINT("[FAT32] ", GFX_GRAY_70);
    BOOTUP_PRINT("searching ", white());
    BOOTUP_PRINT_INT(part_count, white());
    BOOTUP_PRINT(" partitions\n", white());

    for (int i = 0; i < part_count; i++) {
        partition_info_t *part = partition_get_info(i);
        if (part && (part->type == 0x0B || part->type == 0x0C)) {
            // 0x0B = FAT32, 0x0C = FAT32 LBA
            fat32_part = part;
            BOOTUP_PRINT("[FAT32] ", GFX_GRAY_70);
            BOOTUP_PRINT("found FAT32 partition at LBA ", white());
            BOOTUP_PRINT_INT(part->start_lba, white());
            BOOTUP_PRINT("\n", white());
            break;
        }
    }

    if (!fat32_part) {
        BOOTUP_PRINT("[FAT32] ", GFX_GRAY_70);
        BOOTUP_PRINT("no FAT32 partition found\n", red());
        return;
    }

    // use partition start LBA
    partition_start_lba = fat32_part->start_lba;

    BOOTUP_PRINT("[FAT32] ", GFX_GRAY_70);
    BOOTUP_PRINT("read boot sector from LBA ", white());
    BOOTUP_PRINT_INT(partition_start_lba, white());
    BOOTUP_PRINT("...\n", white());

    if (ATAread_sectors(disk_device, partition_start_lba, 1, (u16*)&bootSector) != 0) {
        BOOTUP_PRINT("[FAT32] ", GFX_GRAY_70);
        BOOTUP_PRINT("failed to read boot sector\n", red());
        return;
    }

    /*BOOTUP_PRINT("[FAT32] ", GFX_GRAY_70);
    BOOTUP_PRINT("boot signature: 0x", white());
    BOOTUP_PRINT_INT(((u8*)&bootSector)[510], white());
    BOOTUP_PRINT_INT(((u8*)&bootSector)[511], white());
    BOOTUP_PRINT("\n", white());

    BOOTUP_PRINT("[FAT32] ", GFX_GRAY_70);
    BOOTUP_PRINT("FS Type: ", white());
    char fs_type[9];
    memcpy(fs_type, bootSector.fat_type_label, 8);
    fs_type[8] = '\0';
    BOOTUP_PRINT(fs_type, white());
    BOOTUP_PRINT("\n", white());

    */

    // verify FAT32
    if (memcmp(bootSector.fat_type_label, "FAT32   ", 8) != 0) {
        BOOTUP_PRINT("[FAT32] ", GFX_GRAY_70);
        BOOTUP_PRINT("not a valid FAT32 partition\n", yellow());
        BOOTUP_PRINT("        should have 'FAT32   ' (8 chars), got '", white());
        for (int i = 0; i < 8; i++) {
            char c = bootSector.fat_type_label[i];
            if (c >= 32 && c < 127) {
                char buf[2] = {c, 0};
                BOOTUP_PRINT(buf, white());
            } else {
                BOOTUP_PRINT("?", white());
            }
        }
        BOOTUP_PRINT("'\n", white());
        return;
    }

    BOOTUP_PRINT("[FAT32] ", GFX_GRAY_70);
    BOOTUP_PRINT("found.\n", green());

    FAT = (uint32_t*)klime_create((klime_t*)fs_klime,
                                  bootSector.table_size_32 * bootSector.bytes_per_sector);
    working_buffer = klime_create((klime_t*)fs_klime,
                                 bootSector.sectors_per_cluster * bootSector.bytes_per_sector);

    if (!FAT || !working_buffer) {
        BOOTUP_PRINT("[FAT32] ", GFX_GRAY_70);
        BOOTUP_PRINT("failed: allocate buffers\n", red());
        return;
    }

    // read FAT
    u32 fat_lba = partition_start_lba + bootSector.reserved_sector_count;
    BOOTUP_PRINT("[FAT32] ", GFX_GRAY_70);
    BOOTUP_PRINT("reading FAT from LBA ", white());
    BOOTUP_PRINT_INT(fat_lba, white());
    BOOTUP_PRINT("...\n", white());

    for (u32 i = 0; i < bootSector.table_size_32; i++) {
        ATAread_sectors(disk_device, fat_lba + i, 1,
                       (u16*)((u8*)FAT + i * bootSector.bytes_per_sector));
    }

    fat32_initialized = 1;
    BOOTUP_PRINT("[FAT32] ", GFX_GRAY_70);
    BOOTUP_PRINT("finish\n", green());

    BOOTUP_PRINT("[FAT32] ", GFX_GRAY_70);
    BOOTUP_PRINT("Bytes/sector: ", white());
    BOOTUP_PRINT_INT(bootSector.bytes_per_sector, white());
    BOOTUP_PRINT(", Sectors/cluster: ", white());
    BOOTUP_PRINT_INT(bootSector.sectors_per_cluster, white());
    BOOTUP_PRINT("\n", white());
}

uint32_t get_next_cluster(uint32_t currentCluster)
{
    if (!fat32_initialized) return 0x0FFFFFFF;
    return FAT[currentCluster] & 0x0FFFFFFF;
}

uint32_t cluster_to_Lba(uint32_t cluster)
{
    uint16_t fat_total_size = (bootSector.table_size_32 * bootSector.table_count);
    u32 data_start = partition_start_lba + bootSector.reserved_sector_count + fat_total_size;
    return data_start + (cluster - 2) * bootSector.sectors_per_cluster;
}

void string_to_fatname(const char* name, char* nameOut)
{
    char fatName[12];

    memset(fatName, ' ', sizeof(fatName));
    fatName[11] = '\0';

    const char* ext = NULL;
    for (const char* p = name; *p; p++) {
        if (*p == '.') ext = p;
    }

    if (ext == NULL)
        ext = name + 11;

    for (int i = 0; i < 8 && name[i] && name + i < ext; i++)
        fatName[i] = (name[i] >= 'a' && name[i] <= 'z') ? name[i] - 32 : name[i];

    if (ext != name + 11)
        for (int i = 0; i < 3 && ext[i + 1]; i++)
            fatName[i + 8] = (ext[i+1] >= 'a' && ext[i+1] <= 'z') ? ext[i+1] - 32 : ext[i+1];

    memcpy(nameOut, fatName, 12);
}

uint8_t CalculateLFNChecksum(char *fatName)
{
    uint8_t sum = 0;

    for (int i = 11; i > 0; i--) {
        sum = ((sum & 1) ? 0x80 : 0) + (sum >> 1);
        sum += *fatName++;
    }

    return sum;
}

int strCasecmp(const char *str1, const char *str2)
{
    while (1) {
        char c1 = *str1;
        char c2 = *str2;

        if (c1 >= 'a' && c1 <= 'z') c1 -= 32;
        if (c2 >= 'a' && c2 <= 'z') c2 -= 32;

        if (c1 != c2) return (c1 < c2) ? -1 : 1;
        if (c1 == '\0') return 0;
        str1++;
        str2++;
    }
}

fat_dir_entry_t* fat32_lookup_in_dir(uint32_t first_cluster, char* name)
{
    if (!fat32_initialized) return NULL;

    fat_dir_entry_t* entry = NULL;
    int found = 0;

    enum  {NORMAL_ENTRY, LFN_ENTRY};
    int entry_state = NORMAL_ENTRY;

    uint8_t order;
    uint8_t checksum;
    char lfn_name[256];

    uint32_t current_cluster = first_cluster;

    do

    {
        // Read cluster
        u32 lba = cluster_to_Lba(current_cluster);
        for (u32 i = 0; i < bootSector.sectors_per_cluster; i++) {
            ATAread_sectors(disk_device, lba + i, 1,
                          (u16*)((u8*)working_buffer + i * bootSector.bytes_per_sector));
        }

        fat_dir_entry_t* current_entry;
        int EntryCount = (bootSector.sectors_per_cluster * bootSector.bytes_per_sector) / 32;

        for(int i = 0; i < EntryCount; i++)
        {
            current_entry = (fat_dir_entry_t*)working_buffer + i;

            switch (entry_state)
            {
            case NORMAL_ENTRY:
            _NORMAL_ENTRY:
                if((current_entry->attributes & FAT_ATTR_LFN) == FAT_ATTR_LFN)
                {
                    if(!(((LFN*)current_entry)->order & 0x40))
                        break;

                    entry_state = LFN_ENTRY;
                    memset(lfn_name, 0, 256);
                    order = ((LFN*)current_entry)->order & 0x3f;
                    checksum = ((LFN*)current_entry)->checksum;

                    uint16_t name_13[13];
                    memcpy(name_13, ((LFN*)current_entry)->name1, sizeof(uint16_t) * 5);
                    memcpy(name_13 + 5, ((LFN*)current_entry)->name2, sizeof(uint16_t) * 6);
                    memcpy(name_13 + 11, ((LFN*)current_entry)->name3, sizeof(uint16_t) * 2);

                    uint8_t offset = (order - 1) * 13;
                    for(int j = 0; j < 13; j++)
                        lfn_name[offset + j] = (char)name_13[j];
                }
                else
                {
                    char fatname[12];
                    string_to_fatname(name, fatname);

                    if(memcmp(current_entry->filename, fatname, 11) == 0)
                    {
                        found = 1;
                        entry = (fat_dir_entry_t*)klime_create((klime_t*)fs_klime, sizeof(fat_dir_entry_t));
                        memcpy(entry, current_entry, sizeof(fat_dir_entry_t));
                        break;
                    }
                }
            break;

            case LFN_ENTRY:
                order--;
                if((order == 0) && ((current_entry->attributes & FAT_ATTR_LFN) == FAT_ATTR_LFN))
                {
                    entry_state = NORMAL_ENTRY;
                }
                else if ((order != 0) && ((current_entry->attributes & FAT_ATTR_LFN) != FAT_ATTR_LFN))
                {
                    entry_state = NORMAL_ENTRY;
                    goto _NORMAL_ENTRY;
                }
                else
                {
                    if(order == 0)
                    {
                        entry_state = NORMAL_ENTRY;
                        if(checksum != CalculateLFNChecksum(current_entry->filename))
                            goto _NORMAL_ENTRY;

                        if(strCasecmp(lfn_name, name) == 0)
                        {
                            found = 1;
                            entry = (fat_dir_entry_t*)klime_create((klime_t*)fs_klime, sizeof(fat_dir_entry_t));
                            memcpy(entry, current_entry, sizeof(fat_dir_entry_t));
                            break;
                        }
                        break;
                    }

                    if(((LFN*)current_entry)->order != order)
                    {
                        entry_state = NORMAL_ENTRY;
                        break;
                    }

                    uint16_t name_13[13];
                    memcpy(name_13, ((LFN*)current_entry)->name1, sizeof(uint16_t) * 5);
                    memcpy(name_13 + 5, ((LFN*)current_entry)->name2, sizeof(uint16_t) * 6);
                    memcpy(name_13 + 11, ((LFN*)current_entry)->name3, sizeof(uint16_t) * 2);

                    uint8_t offset = (order - 1) * 13;
                    for(int j = 0; j < 13; j++)
                        lfn_name[offset + j] = (char)name_13[j];
                }
            break;
            }
        }

        current_cluster = get_next_cluster(current_cluster);

    } while (current_cluster < 0x0FFFFFF8 && !found);

    return entry;
}

file_t* fat32_open(const char* path)
{
    if (!fat32_initialized) return NULL;



    fat_dir_entry_t* entry;
    char* name;

    char parsed_path[256];
    str_copy(parsed_path, path);

    // searching in the root dir first
    name = parsed_path;
    while (*name == '/') name++;

    char* next = name;
    while (*next && *next != '/') next++;
    if (*next) *next++ = '\0';

    entry = fat32_lookup_in_dir(bootSector.root_cluster, name);

    if(!entry)
        return NULL;

    while (*next)
    {
        name = next;
        while (*name == '/') name++;
        if (!*name) break;

        next = name;
        while (*next && *next != '/') next++;
        if (*next) *next++ = '\0';

        uint32_t first_cluster = entry->firstClusterLow | (entry->firstClusterHigh << 16);
        klime_free((klime_t*)fs_klime, (u64*)entry);

        entry = fat32_lookup_in_dir(first_cluster, name);

        if(!entry)
            return NULL;
    }

    file_t* this_file = (file_t*)klime_create((klime_t*)fs_klime, sizeof(file_t));
    this_file->entry = entry;
    this_file->readPos = 0;

    return this_file;
}

int fat32_read(file_t* this_file, void* buffer, size_t size)
{
    if (!fat32_initialized) return -1;
    if((this_file->entry->attributes & FAT_ATTR_REGULAR) != FAT_ATTR_REGULAR)
        return -1;

    if (this_file->readPos >= this_file->entry->fileSize)
        return -1;

    size = ((this_file->readPos + size) > this_file->entry->fileSize) ?
           (this_file->entry->fileSize - this_file->readPos) : size;

    uint32_t current_cluster = this_file->entry->firstClusterLow |
                              (this_file->entry->firstClusterHigh << 16);
    uint32_t skipped_clusters = (uint32_t)(this_file->readPos /
                                (bootSector.sectors_per_cluster * bootSector.bytes_per_sector));

    for(int i = 0; i < skipped_clusters; i++)
        current_cluster = get_next_cluster(current_cluster);

    uint32_t offset = this_file->readPos -
                     (skipped_clusters * bootSector.sectors_per_cluster * bootSector.bytes_per_sector);
    size_t to_read = 0;

    while (current_cluster < 0x0FFFFFF8 && to_read < size)
    {
        u32 lba = cluster_to_Lba(current_cluster);
        for (u32 i = 0; i < bootSector.sectors_per_cluster; i++) {
            ATAread_sectors(disk_device, lba + i, 1,
                          (u16*)((u8*)working_buffer + i * bootSector.bytes_per_sector));
        }

        uint16_t byte_to_read = (bootSector.sectors_per_cluster * bootSector.bytes_per_sector) - offset;
        byte_to_read = ((byte_to_read + to_read) > size) ? (size - to_read) : byte_to_read;

        memcpy((u8*)buffer + to_read, (u8*)working_buffer + offset, byte_to_read);

        to_read += byte_to_read;
        offset = 0;
        current_cluster = get_next_cluster(current_cluster);
    }

    this_file->readPos += to_read;
    return to_read;
}

void fat32_close(file_t* file)
{
    if (file) {
        if (file->entry) {
            klime_free((klime_t*)fs_klime, (u64*)file->entry);
        }
        klime_free((klime_t*)fs_klime, (u64*)file);
    }
}

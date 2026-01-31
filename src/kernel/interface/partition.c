#include "partition.h"
#include "mbr.h"
#include <drivers/storage/ata/disk.h>
#include <memory/main.h>
#include <string/string.h>
#include <kernel/communication/serial.h>
#include <config/disk.h>
#include <theme/stdclrs.h>
#include <kernel/graph/theme.h>
#include <theme/doccr.h>

static partition_info_t partitions[MAX_PARTITIONS];
static int partition_count = 0;

int partition_init(void) {
    log("[PARTITION]", "looking for partition.\n", d);

    partition_count = 0;
    memset(partitions, 0, sizeof(partitions));
    /*if (partition_count >= MAX_PARTITIONS) {
        BOOTUP_PRINT("[PARTITION] ", GFX_GRAY_70);
        BOOTUP_PRINT("everything full\n", red());
        return -1;
        }*/


    // read
    u8 mbr_buffer[512];
    mbr_t *mbr = (mbr_t*)mbr_buffer;
    if (mbr_read(mbr_buffer) != 0) {
        log("[PARTITION]", "failed to read\n", error);
        return -1;
    }
    if (!mbr_is_valid(mbr)) {
        log("[PARTITION]", "Invalid MBR signature\n", warning);
        return -1;
    }

    // partition table
    for (int i = 0; i < 4; i++) {
        mbr_partition_t *mbr_part = &mbr->partitions[i];

        if (mbr_part->partition_type == MBR_PARTITION_EMPTY) {
            continue;
        }

        partition_info_t *part = &partitions[partition_count];
        part->valid = 1;
        part->bootable = (mbr_part->status == 0x80) ? 1 : 0;
        part->type = mbr_part->partition_type;
        part->start_lba = mbr_part->first_lba;
        part->sector_count = mbr_part->sector_count;
        str_copy(part->type_name, mbr_get_partition_type_name(mbr_part->partition_type));

        BOOTUP_PRINT("  Partition ", white());
        BOOTUP_PRINT_INT(partition_count, white());
        BOOTUP_PRINT(": ", white());
        BOOTUP_PRINT(part->type_name, cyan());
        BOOTUP_PRINT(" LBA: ", white());
        BOOTUP_PRINT_INT(part->start_lba, white());
        BOOTUP_PRINT(", sectors: ", white());
        BOOTUP_PRINT_INT(part->sector_count, white());
        BOOTUP_PRINT("\n", white());

        partition_count++;
    }

    log("[PARTITION]", "", d);
    char buf[64];
    str_copy(buf, "found ");
    str_append_uint(buf, partition_count);
    str_append(buf, " partitions\n");
    BOOTUP_PRINT(buf, white());

    return 0;
}

int partition_get_count(void) {
    return partition_count;
}

partition_info_t* partition_get_info(int index) {
    if (index < 0 || index >= partition_count) {
        return NULL;
    }
    return &partitions[index];
}

int partition_needs_format(void) {
    // if we have valid fat32
    for (int i = 0; i < partition_count; i++) {
        if (partitions[i].type == MBR_PARTITION_FAT32 ||
            partitions[i].type == MBR_PARTITION_FAT32_LBA) {
            return 0;
            //TODO: prove 0x55AA, "FAT32" and FSinfo
        }
    }
    return 1; // it needs formating if it gets here
}


//not a real format rn but it works... so i don't touch it
int partition_format_disk_fat32(void) {
    BOOTUP_PRINT("[PARTITION] ", GFX_GRAY_70);
    BOOTUP_PRINT("formatting disk with FAT32...\n", yellow());


    // NOTE:
    // this will overwrite everything withouth backups or other
    ATAdevice_t *device = ATAget_device(0);
    if (!device) {
        BOOTUP_PRINT("[PARTITION] ", GFX_GRAY_70);
        BOOTUP_PRINT("nothing foun\n", red());
        return -1;
    }



    mbr_t mbr; // new MBR
    mbr_create_empty(&mbr);


    u32 start_lba = 2048; // size of partition
    u64 total_sectors_64 = device->sectors;
    u32 total_sectors = (u32)total_sectors_64;
    u32 partition_sectors ; //= total_sectors - start_lba - 2048;

    if (total_sectors_64 < (start_lba + 2048)) {
        BOOTUP_PRINT("[PARTITION] ", GFX_GRAY_70);
        BOOTUP_PRINT("disk is too small\n", red());
        return -1;
    }
    if (total_sectors_64 > 0xFFFFFFFF) {
        total_sectors = 0xFFFFFFFF; // Max for MBR
    } else {
        total_sectors = (u32)total_sectors_64;
    }


    partition_sectors = total_sectors - start_lba - 2048;
    if (partition_sectors < PARTSECSIZE) {
        BOOTUP_PRINT("[PARTITION] ", GFX_GRAY_70);
        BOOTUP_PRINT("partition too small\n", red());
        return -1;
    }



    BOOTUP_PRINT("  tot. sectors: ", white());
    BOOTUP_PRINT_INT(total_sectors, white());
    BOOTUP_PRINT("\n  part sectors: ", white());
    BOOTUP_PRINT_INT(partition_sectors, white());
    BOOTUP_PRINT("\n", white());



    if (mbr_create_partition(&mbr, 0, MBR_PARTITION_FAT32_LBA, start_lba, partition_sectors) != 0) {
        BOOTUP_PRINT("[PARTITION] ", GFX_GRAY_70);
        BOOTUP_PRINT("failed to create partition entry\n", red());
        return -1;
    }
    if (mbr_write(&mbr) != 0) {
        BOOTUP_PRINT("[PARTITION] ", GFX_GRAY_70);
        BOOTUP_PRINT("failed to write MBR\n", red());
        return -1;
    }

    BOOTUP_PRINT("[PARTITION] ", GFX_GRAY_70);
    BOOTUP_PRINT("MBR created .\n", green());

    return 0;
}

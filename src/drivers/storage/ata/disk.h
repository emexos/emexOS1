//src/drivers/storage/disk.h
#ifndef DISK_H
#define DISK_H

#include <types.h>
#include <kernel/module/module.h>

#define ATA_PRIMARY_DATA         0x1F0
#define ATA_PRIMARY_ERROR        0x1F1
#define ATA_PRIMARY_FEATURES     0x1F1
#define ATA_PRIMARY_SECTOR_COUNT 0x1F2
#define ATA_PRIMARY_LBA_LOW      0x1F3
#define ATA_PRIMARY_LBA_MID      0x1F4
#define ATA_PRIMARY_LBA_HIGH     0x1F5
#define ATA_PRIMARY_DRIVE_SELECT 0x1F6
#define ATA_PRIMARY_STATUS       0x1F7
#define ATA_PRIMARY_COMMAND      0x1F7
#define ATA_PRIMARY_ALT_STATUS   0x3F6
#define ATA_PRIMARY_CONTROL      0x3F6
#define ATA_SECONDARY_DATA         0x170
#define ATA_SECONDARY_ERROR        0x171
#define ATA_SECONDARY_FEATURES     0x171
#define ATA_SECONDARY_SECTOR_COUNT 0x172
#define ATA_SECONDARY_LBA_LOW      0x173
#define ATA_SECONDARY_LBA_MID      0x174
#define ATA_SECONDARY_LBA_HIGH     0x175
#define ATA_SECONDARY_DRIVE_SELECT 0x176
#define ATA_SECONDARY_STATUS       0x177
#define ATA_SECONDARY_COMMAND      0x177
#define ATA_SECONDARY_ALT_STATUS   0x376
#define ATA_SECONDARY_CONTROL      0x376

// ata Commands
#define ATA_CMD_READ_SECTORS 0x20
#define ATA_CMD_READ_SECTORS_EXT 0x24
#define ATA_CMD_WRITE_SECTORS 0x30
#define ATA_CMD_WRITE_SECTORS_EXT 0x34
#define ATA_CMD_CACHE_FLUSH       0xE7
#define ATA_CMD_IDENTIFY          0xEC

// ata SRB
#define ATA_STATUS_ERR   (1 << 0)  // error
#define ATA_STATUS_IDX   (1 << 1)  // index
#define ATA_STATUS_CORR  (1 << 2)  // correct
#define ATA_STATUS_DRQ   (1 << 3)  // DRR
#define ATA_STATUS_DSC   (1 << 4)  // DSC
#define ATA_STATUS_DF    (1 << 5)  // D fault
#define ATA_STATUS_DRDY  (1 << 6)  // D ready
#define ATA_STATUS_BSY   (1 << 7)  // BSY

// ata CRB
#define ATA_CONTROL_NIEN (1 << 1)  // Disable interrupts
#define ATA_CONTROL_SRST (1 << 2)  // Software reset
#define ATA_CONTROL_HOB  (1 << 7)  // High Order Byte

// ata D select (a/b)
#define ATA_DRIVE_MASTER 0xA0
#define ATA_DRIVE_SLAVE  0xB0

// Sector size
#define ATA_SECTOR_SIZE 512 //(1 sek)

// timed out values(in loop iterations)
#define ATA_TIMEOUT_BSY 10000
#define ATA_TIMEOUT_DRQ 10000


typedef enum {
    ATA_DEVICE_NONE = 0,
    ATA_DEVICE_PATA,
    ATA_DEVICE_SATA,
    ATA_DEVICE_PATAPI,
    ATA_DEVICE_SATAPI
} ATAdevice_type_t;
typedef struct {
    u16 base_port;        // base io port (0x1F0 or 0x170)
    u16 control_port;     // control port (0x3F6 or 0x376)
    u8 is_slave;          // 0 = master, 1 = slave
    ATAdevice_type_t type;// type of devise
    u32 capabilities;     // Device capabilities
    char model[41];       // Model string
    char serial[21];      // Serial number
    u8 lba48_supported;   // LBA48 support flag
    u64 sectors;
} ATAdevice_t;
typedef struct {
    u16 config;              // W0
    u16 cylinders;           // W1
    u16 reserved1;           // W2
    u16 heads;               // W3
    u16 reserved2[2];        // W4-W5
    u16 sectors_per_track;   // W6
    u16 reserved3[3];        // W7-W9
    u16 serial[10];          // W10-W19 (20 ASCII characters)
    u16 reserved4[3];        // W20-W22
    u16 firmware_rev[4];     // W23-W26 (8 ASCII characters)
    u16 model[20];           // W27-W46 (40 ASCII characters)
    u16 max_multiple;        // W47
    u16 reserved5;           // W48
    u16 capabilities[2];     // W49-W50
    u16 reserved6[2];        // W51-W52
    u16 valid_fields;        // W53
    u16 reserved7[5];        // W54-W58
    u16 current_multiple;    // W59
    u16 lba28_sectors[2];    // W60-W61 (addressabel sectors(28))
    u16 reserved8[38];       // W62-W99
    u16 lba48_sectors[4];    // W100-W103 (addressabel sectors(48))
    u16 reserved9[152];      // W104-W255
} __attribute__((packed)) ATAidentify_t;

// pub
void ata_init(void);

int ATAdetect_devices(void);
ATAdevice_t* ATAget_device(int index);
int ATAget_device_count(void);

int ATAread_sectors(ATAdevice_t *dev, u64 lba, u8 sector_count, u16 *buffer);
int ATAwrite_sectors(ATAdevice_t *dev, u64 lba, u8 sector_count, u16 *buffer);

// timeout
int ATAwait_bsy(u16 base);
int ATAwait_drq(u16 base);
u8 ATAstatus_read(u16 base);
int ATAdevice_tidentify(ATAdevice_t *dev, ATAidentify_t *identify);
void ATAstring_swap(char *str, int len);




extern driver_module ata_module;

#endif

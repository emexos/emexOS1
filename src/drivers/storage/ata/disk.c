#include "disk.h"
#include "ide.h"
#include <kernel/include/ports.h>
#include <memory/main.h>
#include <string/string.h>
#include <theme/stdclrs.h>
#include <kernel/graph/theme.h>
#include <theme/doccr.h>
//#include <drivers/drivers.h>

//note:
// this is not a perfect ata driver and will not work on real hardware, but for fat32 i need a disk driver so i chose this one

#define MAX_ATA_DEVICES 4

static ATAdevice_t ATAdevices[MAX_ATA_DEVICES];
static int ATAdevice_count = 0;

int ATAwait_bsy(u16 base) {
    int timeout = ATA_TIMEOUT_BSY * 100; // increase timeout

    while (timeout > 0) {
        u8 status = inb(base + 7);
        if (!(status & ATA_STATUS_BSY)) {
            return 0; //s uccess
        }
        io_wait();
        timeout--;
    }

    return -1; // timed out
}

int ATAwait_drq(u16 base) {
    int timeout = ATA_TIMEOUT_DRQ * 100; // increase timeout

    while (timeout > 0) {
        u8 status = inb(base + 7);
        if (status & ATA_STATUS_DRQ) {
            return 0; // success
        }
        if (status & ATA_STATUS_ERR) {
            u8 err = inb(base + 1); // read error register
            log("[ATA]", "Error: ", error);
            BOOTUP_PRINT_INT(err, white());
            BOOTUP_PRINT("\n", white());
            return -2; // rrror
        }
        io_wait();
        timeout--;
    }

    return -1; // timed out
}


u8 ATAstatus_read(u16 base) {
    return inb(base + 7);
}

void ATAstring_swap(char *str, int len) {
    for (int i = 0; i < len; i += 2) {
        char tmp = str[i];
        str[i] = str[i + 1];
        str[i + 1] = tmp;
    }
}

// 400ns delay when reading alternate status
static void ATA400ns_delay(u16 control_port) {
    for (int i = 0; i < 14; i++) { // increase to 1000ns for safety
        inb(control_port);
    }
}

static void ATAsoft_reset(u16 control_port) { // (software resset)
    outb(control_port, ATA_CONTROL_SRST);
    ATA400ns_delay(control_port);
    outb(control_port, 0);
    ATA400ns_delay(control_port);
}

// ATA IDENTIFY command
int ATAidentify(ATAdevice_t *dev, ATAidentify_t *identify)
{
    u16 base = dev->base_port;
    u8 drive = dev->is_slave ? ATA_DRIVE_SLAVE : ATA_DRIVE_MASTER;

    // floating bus
    u8 status = inb(base + 7);
    if (status == 0xFF) {
        return -1; // floating bus and no device/S
    }

    // select drive
    outb(base + 6, drive);
    ATA400ns_delay(dev->control_port);

    // read status after selection
    status = inb(base + 7);
    if (status == 0 || status == 0xFF) {
        return -1; // no drive
    }

    // disable interrupts
    outb(dev->control_port, ATA_CONTROL_NIEN);

    // set sector count and LBA to 0
    outb(base + 2, 0);
    outb(base + 3, 0);
    outb(base + 4, 0);
    outb(base + 5, 0);

    // send ATA IDENTIFY command
    outb(base + 7, ATA_CMD_IDENTIFY);
    ATA400ns_delay(dev->control_port);

    // checks if drive exists
    status = inb(base + 7);
    if (status == 0) {
        return -1; // no drive
    }
    if (ATAwait_bsy(base) != 0) {
        BOOTUP_PRINT("    Timeout waiting for BSY\n", GFX_RED);
        return -1;
    }

    // looks for ATAPI device
    u8 lba_mid = inb(base + 4);
    u8 lba_high = inb(base + 5);

    if (lba_mid != 0 || lba_high != 0) {
        // ATAPI device
        dev->type = (lba_mid == 0x14 && lba_high == 0xEB) ?
                    ATA_DEVICE_PATAPI : ATA_DEVICE_SATAPI;
        BOOTUP_PRINT("    ATAPI devices are not supported", red());
        return -2; // not supported yet
    }

    // waits until DRQ with timeout
    int drq_result = ATAwait_drq(base);
    if (drq_result != 0) {
        if (drq_result == -2) {
            BOOTUP_PRINT("    Error flag set\n", red());
        } else {
            BOOTUP_PRINT("    Timeout waiting for DRQ\n", red());
        }
        return -1;
    }

    // reads 256 words (512 bytes, 1sek)
    u16 *buffer = (u16 *)identify;
    for (int i = 0; i < 256; i++) {
        buffer[i] = inw(base);
    }

    return 0;
}

// detects ata device on specific bus
static int ATAdetect_device(u16 base_port, u16 control_port, u8 is_slave)
{
    if (ATAdevice_count >= MAX_ATA_DEVICES) {
        return -1;
    }

    // creates a temporary device structure
    ATAdevice_t temp_dev;
    memset(&temp_dev, 0, sizeof(ATAdevice_t));
    temp_dev.base_port = base_port;
    temp_dev.control_port = control_port;
    temp_dev.is_slave = is_slave;
    temp_dev.type = ATA_DEVICE_NONE;

    // this checks if the ports are responding
    u8 initial_status = inb(base_port + 7);
    if (initial_status == 0xFF) {
        return -1; // floating bus
    }

    ATAidentify_t identify;
    int result = ATAidentify(&temp_dev, &identify);
    if (result != 0) {
        return -1;
    }



    ATAdevice_t *dev = &ATAdevices[ATAdevice_count];
    memcpy(dev, &temp_dev, sizeof(ATAdevice_t));
    memcpy(dev->model, identify.model, 40);
    ATAstring_swap(dev->model, 40);
    dev->model[40] = '\0';

    // trimt railing spaces
    for (int i = 39; i >= 0; i--) {
        if (dev->model[i] == ' ') {
            dev->model[i] = '\0';
        } else {
            break;
        }
    }

    // identifis serial number
    memcpy(dev->serial, identify.serial, 20);
    ATAstring_swap(dev->serial, 20);
    dev->serial[20] = '\0';

    // trim trailing spaces from serial
    for (int i = 19; i >= 0; i--) {
        if (dev->serial[i] == ' ') {
            dev->serial[i] = '\0';
        } else {
            break;
        }
    }

    // LBA48 support
    dev->lba48_supported = (identify.capabilities[1] & (1 << 10)) ? 1 : 0;



    if (dev->lba48_supported)
    {
        dev->sectors = ((u64)identify.lba48_sectors[3] << 48) |
                      ((u64)identify.lba48_sectors[2] << 32) |
                      ((u64)identify.lba48_sectors[1] << 16) |
                      ((u64)identify.lba48_sectors[0]);
    } else {
        dev->sectors = ((u32)identify.lba28_sectors[1] << 16) |
                      ((u32)identify.lba28_sectors[0]);
    }


    dev->type = ATA_DEVICE_PATA;
    dev->capabilities = identify.capabilities[0];

    ATAdevice_count++;
    return 0;
}

int ATAdetect_devices(void)
{
    ATAdevice_count = 0;

    log("[ATA]", "Detecting devices...\n", d);

    // PCI-IDE ports (not legacy!)
    u16 primary_base = pci_ide_get_primary_base();
    u16 primary_ctrl = pci_ide_get_primary_ctrl();
    u16 secondary_base = pci_ide_get_secondary_base();
    u16 secondary_ctrl = pci_ide_get_secondary_ctrl();

    // primary
    BOOTUP_PRINT("      Checking Primary Master... ", white());
    if (ATAdetect_device(primary_base, primary_ctrl, 0) == 0) {
        BOOTUP_PRINT(" -> Found: ", white());
        BOOTUP_PRINT(ATAdevices[ATAdevice_count - 1].model, white());
        BOOTUP_PRINT("\n", white());
    } else {
        BOOTUP_PRINT(" -> None\n", white());
    }

    // primary slave
    BOOTUP_PRINT("      Checking Primary Slave... ", white());
    if (ATAdetect_device(primary_base, primary_ctrl, 1) == 0) {
        BOOTUP_PRINT(" -> Found: ", white());
        BOOTUP_PRINT(ATAdevices[ATAdevice_count - 1].model, white());
        BOOTUP_PRINT("\n", white());
    } else {
        BOOTUP_PRINT(" -> None\n", white());
    }

    // secondary master
    BOOTUP_PRINT("      Checking Secondary Master... ", white());
    if (ATAdetect_device(secondary_base, secondary_ctrl, 0) == 0) {
        BOOTUP_PRINT(" -> Found: ", white());
        BOOTUP_PRINT(ATAdevices[ATAdevice_count - 1].model, white());
        BOOTUP_PRINT("\n", white());
    } else {
        BOOTUP_PRINT(" -> None\n", white());
    }

    // secondary slave
    BOOTUP_PRINT("      Checking Secondary Slave... ", white());
    if (ATAdetect_device(secondary_base, secondary_ctrl, 1) == 0) {
        BOOTUP_PRINT(" -> Found: ", white());
        BOOTUP_PRINT(ATAdevices[ATAdevice_count - 1].model, white());
        BOOTUP_PRINT("\n", white());
    } else {
        BOOTUP_PRINT(" -> None\n", white());
    }

    char buf[64];
    log("[ATA]", "", d);
    str_copy(buf, "Found ");
    str_append_uint(buf, ATAdevice_count);
    str_append(buf, " device(s)\n");
    BOOTUP_PRINT(buf, white());

    return ATAdevice_count;
}

// LBA28 read sector
int ATAread_sectors(ATAdevice_t *dev, u64 lba, u8 sector_count, u16 *buffer)
{
    u16 base = dev->base_port;
    u8 drive = dev->is_slave ? ATA_DRIVE_SLAVE : ATA_DRIVE_MASTER;
    if (!dev || dev->type == ATA_DEVICE_NONE) {
        log("[ATA]", "Invalid device\n", error);
        return -1;
    }
    if (sector_count == 0) {
        log("[ATA]", "Zero sectors requested\n", error);
        return -1;
    }

    if (ATAwait_bsy(base) != 0) {
        log("[ATA]", "Initial BSY timeout\n", error);
        return -1;
    }

    // disables interups
    outb(dev->control_port, ATA_CONTROL_NIEN);
    ATA400ns_delay(dev->control_port);

    // select drive with right LBA mode
    outb(base + 6, drive | 0xE0 | ((lba >> 24) & 0x0F)); // 0xE0 is LBA
    ATA400ns_delay(dev->control_port);

    // waits abit
    if (ATAwait_bsy(base) != 0) {
        log("[ATA]", "BSY waiting\n", error);
        return -1;
    }

    outb(base + 2, sector_count);
    outb(base + 3, (u8)(lba));
    outb(base + 4, (u8)(lba >> 8));
    outb(base + 5, (u8)(lba >> 16));
    outb(base + 7, ATA_CMD_READ_SECTORS);
    ATA400ns_delay(dev->control_port);

    for (int sector = 0; sector < sector_count; sector++)
    {
        // BSY clear
        if (ATAwait_bsy(base) != 0) {
            log("[ATA]", "BSY timeout in sector loop at sector ", error);
            BOOTUP_PRINT_INT(sector, red());
            BOOTUP_PRINT("\n", red());
            return -1;
        }

        // DRQ
        int drq_result = ATAwait_drq(base);
        if (drq_result != 0) {
            log("[ATA]", "DRQ timeout at sector ", error);
            BOOTUP_PRINT_INT(sector, red());
            BOOTUP_PRINT(", status: ", red());
            u8 status = ATAstatus_read(base);
            BOOTUP_PRINT_INT(status, red());
            BOOTUP_PRINT("\n", red());
            return -1;
        }


        u8 status = ATAstatus_read(base);
        if (status & ATA_STATUS_ERR) {
            u8 err = inb(base + 1);
            log("[ATA]", "Error status at sector ", error);
            BOOTUP_PRINT_INT(sector, red());
            BOOTUP_PRINT(", error code: ", red());
            BOOTUP_PRINT_INT(err, red());
            BOOTUP_PRINT("\n", red());
            return -1;
        }

        // 256 words (512 bytes, 1 sek)
        for (int i = 0; i < 256; i++) {
            buffer[sector * 256 + i] = inw(base);
        }




        ATA400ns_delay(dev->control_port);
    }
return 0;
}
int ATAwrite_sectors(ATAdevice_t *dev, u64 lba, u8 sector_count, u16 *buffer)
{
    u16 base = dev->base_port;
    u8 drive = dev->is_slave ? ATA_DRIVE_SLAVE : ATA_DRIVE_MASTER;
    if (!dev || dev->type == ATA_DEVICE_NONE) {
        return -1;
    }
    if (sector_count == 0) {
        return -1;
    }
    if (ATAwait_bsy(base) != 0) {
        return -1;
    }

    // disables intersupts
    outb(dev->control_port, ATA_CONTROL_NIEN);
    ATA400ns_delay(dev->control_port);

    // selects lba mode with 0xE0
    outb(base + 6, drive | 0xE0 | ((lba >> 24) & 0x0F));
    ATA400ns_delay(dev->control_port);

    // waits after it
    if (ATAwait_bsy(base) != 0) {
        return -1;
    }


    outb(base + 2, sector_count);
    outb(base + 3, (u8)(lba));
    outb(base + 4, (u8)(lba >> 8));
    outb(base + 5, (u8)(lba >> 16));
    outb(base + 7, ATA_CMD_WRITE_SECTORS);
    ATA400ns_delay(dev->control_port);

    for (int sector = 0; sector < sector_count; sector++)
    {
        if (ATAwait_bsy(base) != 0) {
            return -1;
        }
        if (ATAwait_drq(base) != 0) {
            return -1;
        }
        u8 status = ATAstatus_read(base);
        if (status & ATA_STATUS_ERR) {
            return -1;
        }

        // writes 256 words (512 bytes, 1 sek)
        for (int i = 0; i < 256; i++) {
            outw(base, buffer[sector * 256 + i]);
        }

        ATA400ns_delay(dev->control_port);
    }

    //flush cache
    if (ATAwait_bsy(base) == 0) {
        outb(base + 7, ATA_CMD_CACHE_FLUSH);
        ATAwait_bsy(base);
    }
    return 0;
}

// get device by index
ATAdevice_t* ATAget_device(int index) {
    if (index < 0 || index >= ATAdevice_count) {
        return NULL;
    }
    return &ATAdevices[index];
}
int ATAget_device_count(void) {
    return ATAdevice_count;
}

/*

static int ATAmodule_init(void) {
    //ATAdetect_devices();
    // already done
    log("[ATA]", "Load ATA module...\n", d);
    return 0;
}*/

// pub:
void ata_init(void) {
    log("[ATA]", "Init ATA driver...\n", d);

    // Initialize PCI IDE controller FIRST
    if (pci_find_ide_controller() == 0) {
        log("[ATA]", "No PCI IDE controller found, using legacy ports\n", warning);
    }

    //ATAmodule_init();
    ATAdetect_devices();
}
/*
static void ATAmodule_fini(void) {
    // cleanup if needed
}

driver_module ata_module = {
    .name = ATANAME,
    .mount = ATAPATH,
    .version = ATAUNIVERSAL,
    .init = ATAmodule_init,
    .fini = ATAmodule_fini,
    .open = NULL,
    .read = NULL,
    .write = NULL,
};
*/
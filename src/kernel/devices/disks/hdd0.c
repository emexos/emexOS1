#include "hdd0.h"
#include <kernel/module/module.h>
#include <kernel/communication/serial.h>
#include <string/string.h>
#include <theme/doccr.h>
#include <drivers/drivers.h>
#include <memory/main.h>
#include <types.h>

#define HDA_SECTOR_SIZE 512

static int ATAmodule_init(void) {
    log("[HDA]", "init /dev/hda\n", d);
    return 0;
}

static void ATAmodule_fini(void) {
    // cleanup if needed
}

static void *hda_open(const char *path) {
    (void)path;
    return ATAget_device(0);
}

static int hda_read(void *handle, void *buf, size_t count, u64 offset)
{
    ATAdevice_t *dev = (ATAdevice_t *)handle;
    if (!dev || !buf || count == 0) return -1;

    u64 lba = offset / HDA_SECTOR_SIZE;
    u32 lba_offset = offset % HDA_SECTOR_SIZE;
    u32 sectors = (u32)((lba_offset + count + HDA_SECTOR_SIZE - 1) / HDA_SECTOR_SIZE);

    // temp buffer for unaligned reads
    static u8 tmp[HDA_SECTOR_SIZE] __attribute__((aligned(16)));
    size_t remaining = count;
    u8 *dst = (u8 *)buf;

    for (u32 i = 0; i < sectors && remaining > 0; i++)
    {
        if (ATAread_sectors(dev, lba + i, 1, (u16 *)tmp) != 0) return -1;

        u32 src_off = (i == 0) ? lba_offset : 0;
        u32 to_copy = HDA_SECTOR_SIZE - src_off;
        if (to_copy > remaining) to_copy = (u32)remaining;

        memcpy(dst, tmp + src_off, to_copy);

        dst += to_copy;
        remaining -= to_copy;
    }
    return (int)(count - remaining);
}

static int hda_write(void *handle, const void *buf, size_t count, u64 offset)
{
    ATAdevice_t *dev = (ATAdevice_t *)handle;
    if (!dev || !buf || count == 0) return -1;

    u64 lba = offset / HDA_SECTOR_SIZE;
    u32 lba_offset = offset % HDA_SECTOR_SIZE;
    u32 sectors = (u32)((lba_offset + count + HDA_SECTOR_SIZE - 1) / HDA_SECTOR_SIZE);

    static u8 tmp[HDA_SECTOR_SIZE] __attribute__((aligned(16)));
    size_t remaining = count;
    const u8 *src = (const u8 *)buf;

    for (u32 i = 0; i < sectors && remaining > 0; i++)
    {
        u32 dst_off = (i == 0) ? lba_offset : 0;
        u32 to_copy = HDA_SECTOR_SIZE - dst_off;
        if (to_copy > remaining) to_copy = (u32)remaining;
        if (dst_off != 0 || to_copy != HDA_SECTOR_SIZE) {
            if (ATAread_sectors(dev, lba + i, 1, (u16 *)tmp) != 0) return -1;
        }

        memcpy(tmp + dst_off, src, to_copy);

        if (ATAwrite_sectors(dev, lba + i, 1, (u16 *)tmp) != 0) return -1;

        src += to_copy;
        remaining -= to_copy;
    }
    return (int)(count - remaining);
}

driver_module ata_module = {
    .name    = ATANAME,
    .mount   = ATAPATH,
    .version = ATAUNIVERSAL,
    .init    = ATAmodule_init,
    .fini    = ATAmodule_fini,
    .open    = hda_open,
    .read    = hda_read,
    .write   = hda_write,
};
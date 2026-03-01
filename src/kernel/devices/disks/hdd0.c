#include "hdd0.h"

#include <kernel/module/module.h>
#include <kernel/graph/graphics.h>
#include <kernel/communication/serial.h>
#include <string/string.h>
#include <theme/doccr.h>
#include <types.h>

#include <drivers/drivers.h>

static int ATAmodule_init(void) {
    //ATAdetect_devices();
    // already done
    log("[ATA]", "Load ATA module...\n", d);
    return 0;
}

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

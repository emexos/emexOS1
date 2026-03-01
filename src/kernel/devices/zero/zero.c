#include "zero.h"
#include <kernel/module/module.h>
#include <kernel/graph/graphics.h>
#include <kernel/communication/serial.h>
#include <theme/doccr.h>
#include <string/string.h>

#include <drivers/drivers.h>

static int zero_mod_init(void) {
    log("[ZERO]", "init /dev/zero\n", d);
    return 0;
}

static void zero_mod_fini(void) {}

static void *zero_open(const char *path) {
    (void)path;
    return (void *)1;
}

// write buffer full with 0s
static int zero_read(void *handle, void *buf, size_t count) {
    (void)handle;
    if (buf && count > 0) {
        memset(buf, 0, count);
    }
    return (int)count; // show read bytes
}

static int zero_write(void *handle, const void *buf, size_t count) {
    (void)handle; (void)buf;
    return (int)count;
}

driver_module zero_module = {
    .name    = ZERNAME,
    .mount   = ZERPATH,
    .version = ZERUNIVERSAL,
    .init    = zero_mod_init,
    .fini    = zero_mod_fini,
    .open    = zero_open,
    .read    = zero_read,
    .write   = zero_write,
};
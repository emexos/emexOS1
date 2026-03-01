#include "null.h"
#include <kernel/module/module.h>
#include <kernel/communication/serial.h>
#include <theme/doccr.h>
#include <drivers/drivers.h>

static int null_mod_init(void) {
    log("[NULL]", "init /dev/null\n", d);
    return 0;
}

static void null_mod_fini(void) {}

static void *null_open(const char *path) {
    (void)path;
    return (void *)1;
}

static int null_read(void *handle, void *buf, size_t count) {
    (void)handle; (void)buf; (void)count;
    // eof
    return 0;
}

static int null_write(void *handle, const void *buf, size_t count) {
    (void)handle; (void)buf;
    return (int)count;
}

driver_module null_module = {
    .name    = NULNAME,
    .mount   = NULPATH,
    .version = NULUNIVERSAL,
    .init    = null_mod_init,
    .fini    = null_mod_fini,
    .open    = null_open,
    .read    = null_read,
    .write   = null_write,
};
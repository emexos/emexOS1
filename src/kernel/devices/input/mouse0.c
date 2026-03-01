#include "mouse0.h"
#include <kernel/module/module.h>
#include <kernel/communication/serial.h>
#include <theme/doccr.h>
#include <drivers/drivers.h>

static int mouse0_init(void) {
    log("[MOUSE]", "init /dev/input/mouse0\n", d);
    return 0;
}

static void mouse0_fini(void) {}

static void *mouse0_open(const char *path) {
    (void)path;
    return (void *)1;
}

static int mouse0_read(void *handle, void *buf, size_t count) {
    (void)handle; (void)buf; (void)count;
    return 0;
}

static int mouse0_write(void *handle, const void *buf, size_t count) {
    (void)handle; (void)buf; (void)count;
    return -1;
}

driver_module mouse0_module = {
    .name    = MS0NAME,
    .mount   = MS0PATH,
    .version = MS0UNIVERSAL,
    .init    = mouse0_init,
    .fini    = mouse0_fini,
    .open    = mouse0_open,
    .read    = mouse0_read,
    .write   = mouse0_write,
};
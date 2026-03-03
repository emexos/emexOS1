#include "mouse0.h"
#include <kernel/module/module.h>
#include <kernel/communication/serial.h>
#include <theme/doccr.h>
#include <drivers/drivers.h>
#include <kernel/arch/x86_64/exceptions/irq.h>
//#include <drivers/ps2/mouse/mouse.h>

static int mouse0_init(void) {
    log("[MOUSE]", "init /dev/input/mouse0\n", d);
    //mouse_init();
    return 0;
}

static void mouse0_fini(void) {
    //irq_unregister_handler(12);
}

static void *mouse0_open(const char *path) {
    (void)path;
    return (void *)1;
}

static int mouse0_read(void *handle, void *buf, size_t count) {
    (void)handle;

/*
	size_t ev_size = sizeof(mouse_event_t);
    size_t written = 0;
    u8 *out = (u8 *)buf;

    while (written + ev_size <= count && mouse_has_event()) {
        mouse_event_t ev;
        if (mouse_get_event(&ev)) {
            u8 *src = (u8 *)&ev;
            for (size_t i = 0; i < ev_size; i++) {
                out[written + i] = src[i];
            }
            written += ev_size;
        }
    }

    return (int)written; */

	return 0;
}

// no write for input devices
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
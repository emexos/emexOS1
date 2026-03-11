#include "mouse0.h"
#include <kernel/module/module.h>
#include <kernel/communication/serial.h>
#include <theme/doccr.h>
#include <drivers/drivers.h>
#include <kernel/arch/x86_64/exceptions/irq.h>
#include <drivers/ps2/mouse/mouse.h>

static int mouse0_init(void) {
    log("[MOUSE]", "init /dev/input/mouse0\n", d);
    mouse_init();
    return 0;
}
static void mouse0_fini(void){
	irq_unregister_handler(12);
}
static void *mouse0_open(const char *p){
	(void)p; return (void *)1;
}
static int mouse0_write(void *h, const void *b, size_t c, u64 o){
	(void)h;(void)b;(void)c;(void)o; return -1;
}

static int mouse0_read(void *handle, void *buf, size_t count, u64 offset) {
    (void)handle; (void)offset;
    size_t esz = sizeof(mouse_event_t);
    size_t written = 0;
    u8 *out = (u8 *)buf;
    while (written + esz <= count && mouse_has_event()) {
        mouse_event_t ev;
        if (mouse_get_event(&ev)) {
            u8 *src = (u8 *)&ev;
            for (size_t i = 0; i < esz; i++) out[written + i] = src[i];
            written += esz;
        }
    }
    return (int)written;
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
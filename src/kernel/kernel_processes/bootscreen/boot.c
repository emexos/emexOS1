#include "boot.h"
#include <kernel/communication/serial.h>

// all the print function will be migrated to this file
// because currently the console is a part of the screen (like console_scrollup)
// and this is bad thats why i create kernel processes and the bootscreen process
// which will handle all bootup messages and also the scroll up (and mb login)

static int bs_init(kproc_t *self) {
    (void)self;
    printf("[BOOTSCREEN] init\n");
    return KPROC_EFINE;
}

static int bs_tick(kproc_t *self) {
    (void)self;
    return KPROC_EFINE;
}

static void bs_fini(kproc_t *self) {
    (void)self;
}

kproc_t bootscreen_proc = {
    .name     = "bootscreen",
    .state    = KPROC_STATE_READY,
    .flags    = KPROC_FLAG_EARLY | KPROC_FLAG_CRITICAL | KPROC_FLAG_PERMANENT,
    .priority = 128,
    .init     = bs_init,
    .tick     = bs_tick,
    .fini     = bs_fini,
};

void init_bootscreen(void) {
    kproc_register_and_start(&bootscreen_proc);
}

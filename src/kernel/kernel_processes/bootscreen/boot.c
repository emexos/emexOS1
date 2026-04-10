#include "boot.h"
#include <kernel/communication/serial.h>
#include <kernel/graph/graphics.h>

// all the print function will be migrated to this file
// because currently the console is a part of the screen (like console_scrollup)
// and this is bad thats why i create kernel processes and the bootscreen process
// which will handle all bootup messages and also the scroll up (and mb login)

bs_screen_t bs_screens[BS_MAX_SCREENS];
int bs_active = 0;

void bs_init_screens(void)
{
	for (int i = 0; i < BS_MAX_SCREENS; i++)
	{
		bs_screens[i].cursor_x = 0;
		bs_screens[i].cursor_y = 0;
		bs_screens[i].buffer = framebuffer; // shared
	}
}

void bs_switch(int id)
{
	if (id < 0 || id >= BS_MAX_SCREENS) return;
	bs_active = id;
}

bs_screen_t* bs_get_active(void)
{
	return &bs_screens[bs_active];
}


static int bs_init(kproc_t *self) {
    (void)self;
    bs_switch(0);
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

kproc_t bootscreen_proc =
{
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

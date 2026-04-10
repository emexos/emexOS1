#ifndef BOOT_H
#define BOOT_H

#include <kernel/kernel_processes/loader.h>

extern kproc_t bootscreen_proc;

// bootscreen tty's
#define BS_MAX_SCREENS 4
#define BS1 "bs1"
#define BS2 "bs2"
#define BS3 "bs3"
#define BS4 "bs4"

typedef struct {
	u32 cursor_x;
	u32 cursor_y;
	u32 *buffer; // opt. buffering
} bs_screen_t;

extern bs_screen_t bs_screens[BS_MAX_SCREENS];
extern int bs_active;


void init_bootscreen(void);
void bs_init_screens(void);
void bs_switch(int id);
bs_screen_t* bs_get_active(void);

#endif

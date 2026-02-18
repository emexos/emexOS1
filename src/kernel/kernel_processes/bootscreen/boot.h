#ifndef BOOT_H
#define BOOT_H

#include <kernel/kernel_processes/loader.h>

extern kproc_t bootscreen_proc;

void init_bootscreen(void);

#endif

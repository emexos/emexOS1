#ifndef DEVICE_TTY0_H
#define DEVICE_TTY0_H

#include <kernel/module/module.h>
#include "tty.h"

#define KEYBOARD0 KBDPATH
//#define MOUSE0 MS0PATH


extern driver_module tty0_module;


void tty0_write_char(char c);
void tty0_set_echo_mode(int mode);
int  tty0_get_echo_mode(void);

#endif
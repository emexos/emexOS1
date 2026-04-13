#ifndef DEVICE_TTY1_H
#define DEVICE_TTY1_H

#include <kernel/module/module.h>

extern driver_module tty1_module;

void tty1_write_char(char c);
void tty1_set_echo_mode(int mode);
int tty1_get_echo_mode(void);

#endif
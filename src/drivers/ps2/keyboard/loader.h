// loader.h
#ifndef KEYBOARD_LOADER_H
#define KEYBOARD_LOADER_H

#include <types.h>
#include "maps.h"

int keymap_load_from_module(const char *name, keymap_t *km);
int keymap_modules_init(void);

#endif

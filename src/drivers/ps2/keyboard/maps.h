#ifndef KEYBOARD_MAPS_H
#define KEYBOARD_MAPS_H

#include <types.h>

typedef struct {
    u8 normal[128];  // normal key mapping
    u8 shift[128];   // shift key mapping
} keymap_t;


int keymap_init(void);
const keymap_t* keymap_get_current(void);
const char* keymap_get_current_name(void);
int keymap_set(const char *name);

#endif

#ifndef KEYBOARD_MAPS_H
#define KEYBOARD_MAPS_H

#include <types.h>

typedef struct {
    u8 normal[128];  // Normal key mapping
    u8 shift[128];   // Shift key mapping
} keymap_t;

// Initialize keymap system
int keymap_init(void);

// Get current keymap
const keymap_t* keymap_get_current(void);

// Get current keymap name
const char* keymap_get_current_name(void);

// Set keymap by name
int keymap_set(const char *name);

#endif

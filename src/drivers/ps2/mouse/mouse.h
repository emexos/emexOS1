#pragma once
#include <types.h>

#define MOUSE_BUF_SIZE 64

#define PS2_DATA 0x60
#define PS2_CMD  0x64

typedef struct {
    int dx, dy; // relative movements
    int abs_x, abs_y;
    unsigned char buttons;
} mouse_event_t;

#define MOUSE_BTN_LEFT   (1<<0)
#define MOUSE_BTN_RIGHT  (1<<1)
#define MOUSE_BTN_MIDDLE (1<<2)

void mouse_init(void);
int mouse_has_event(void);
int mouse_get_event(mouse_event_t *ev);
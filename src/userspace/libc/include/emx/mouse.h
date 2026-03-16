#pragma once

//int mouse_init(void);

#define MOUSE_BTN_LEFT 0x01
#define MOUSE_BTN_RIGHT 0x02

typedef struct {
    int dx, dy;
    int abs_x, abs_y;
    unsigned char buttons;
} mouse_event_t;


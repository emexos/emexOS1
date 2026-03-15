#pragma once

//int mouse_init(void);

typedef struct {
    int dx, dy;
    int abs_x, abs_y;
    unsigned char buttons;
} mouse_event_t;


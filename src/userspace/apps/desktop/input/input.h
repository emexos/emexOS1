#pragma once

#include <sys/types.h>
#include <emx/mouse.h>
#include "../win/win.h"

typedef struct
{
    int valid;
    pid_t pid;
    int wx, wy, ww, wh;
} drag_info_t;

typedef struct
{
    int cx, cy;
    int win_changed;
    drag_info_t drag_prev;
} input_state_t;

void input_frame_begin(input_state_t *is);

void input_init(void);
int input_drain(int mfd, input_state_t *is);
#pragma once

#include <sys/types.h>
#include <emx/mouse.h>
#include "../win/win.h"

typedef struct
{
    int cx, cy;
    int win_changed;
} input_state_t;

void input_frame_begin(input_state_t *is);

void input_init(void);
int input_drain(int mfd, input_state_t *is);
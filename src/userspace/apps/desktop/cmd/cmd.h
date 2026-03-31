#pragma once

#include "../input/input.h"

#define CMD_DIRTY_MAX 8

typedef struct {
    drag_info_t rects[CMD_DIRTY_MAX];
    int count;
    int win_changed;
} cmd_result_t;

void cmd_process(cmd_result_t *result);
int cmd_check_dirty(void);
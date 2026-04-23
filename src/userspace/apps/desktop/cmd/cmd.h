#pragma once

#include "../input/input.h"

typedef struct {
    int win_changed;
} cmd_result_t;

void cmd_process(cmd_result_t *result);
int cmd_check_dirty(void);
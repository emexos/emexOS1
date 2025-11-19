#ifndef GLIME_H
#define GLIME_H

#include <types.h>
#include <../mem.h>

typedef struct glime {
    glime_response_t glres;
    u64 cursor_x;
    u64 cursor_y;
} glime_t;

glime_t *glime_init(glime_response_t gr, u64 *ptr, u64 size);

#endif


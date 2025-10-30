#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "../../../../shared/types.h"

#define KEY_BUFFER_SIZE 128

typedef struct {
    char buffer[KEY_BUFFER_SIZE];
    int read_pos;
    int write_pos;
} key_buffer_t;

void keyboard_init(void);
void keyboard_poll(void);
int keyboard_has_key(void);
char keyboard_get_key(void);

#endif

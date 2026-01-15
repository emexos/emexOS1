#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <types.h>
#include <kernel/module/module.h>

#define KEY_BUFFER_SIZE 128

#define KEY_CTRL_MASK 0x80000000
#define KEY_SHIFT_MASK 0x40000000
#define KEY_ALT_MASK 0x20000000

// key event structure
typedef struct {
    u32 keycode;  // ASCII char or special code
    u32 modifiers; // CTRL, SHIFT, ALT flags
    u8 pressed;    // 1 = pressed, 0 = released
} key_event_t;

typedef struct {
    key_event_t buffer[KEY_BUFFER_SIZE];
    int read_pos;
    int write_pos;
    int count;
} key_buffer_t;

void keyboard_init(void);
void keyboard_poll(void);
int keyboard_has_key(void);
int keyboard_get_event(key_event_t *event);
char keyboard_get_key(void);

extern driver_module keyboard_module;

#endif

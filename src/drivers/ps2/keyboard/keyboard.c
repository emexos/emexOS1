#include "keyboard.h"
#include "maps.h"
#include <kernel/include/ports.h>
#include <kernel/exceptions/irq.h>

static key_buffer_t key_buffer = {0};
static int shift = 0;
static int ctrl = 0;
static int alt = 0;
static int caps = 0;

static void keyboard_put_event(key_event_t event) {
    if (key_buffer.count >= KEY_BUFFER_SIZE) return;

    key_buffer.buffer[key_buffer.write_pos] = event;
    key_buffer.write_pos = (key_buffer.write_pos + 1) % KEY_BUFFER_SIZE;
    key_buffer.count++;
}

static void keyboard_irq_handler(cpu_state_t* state) {
    (void)state;

    if ((inb(0x64) & 1) == 0) return;

    u8 sc = inb(0x60);
    if (sc == 0) return;

    const keymap_t *keymap = keymap_get_current();

    // Handle key release
    if (sc & 0x80) {
        u8 make = sc & 0x7F;
        if (make == 0x2A || make == 0x36) shift = 0;
        if (make == 0x1D) ctrl = 0;
        if (make == 0x38) alt = 0;
        return;
    }

    // Handle modifier keys
    if (sc == 0x2A || sc == 0x36) { shift = 1; return; }
    if (sc == 0x1D) { ctrl = 1; return; }
    if (sc == 0x38) { alt = 1; return; }
    if (sc == 0x3A) { caps = !caps; return; }

    // Create key event
    key_event_t event;
    event.pressed = 1;
    event.modifiers = 0;
    if (ctrl) event.modifiers |= KEY_CTRL_MASK;
    if (shift) event.modifiers |= KEY_SHIFT_MASK;
    if (alt) event.modifiers |= KEY_ALT_MASK;

    // Special keys
    if (sc == 0x0E) {
        event.keycode = '\b';
        keyboard_put_event(event);
        return;
    }
    if (sc == 0x1C) {
        event.keycode = shift ? '\r' : '\n';
        keyboard_put_event(event);
        return;
    }

    // Convert scancode to character
    if (sc < 128) {
        u8 c = shift ? keymap->shift[sc] : keymap->normal[sc];

        if (c >= 'a' && c <= 'z') {
            if ((caps && !shift) || (!caps && shift)) {
                c = (c - 'a') + 'A';
            }
        }

        if (c) {
            event.keycode = c;
            keyboard_put_event(event);
        }
    }
}

void keyboard_init(void) {
    keymap_init();
    key_buffer.read_pos = 0;
    key_buffer.write_pos = 0;
    key_buffer.count = 0;

    irq_register_handler(1, keyboard_irq_handler);
}

int keyboard_has_key(void) {
    return key_buffer.count > 0;
}

int keyboard_get_event(key_event_t *event) {
    if (key_buffer.count == 0) return 0;

    *event = key_buffer.buffer[key_buffer.read_pos];
    key_buffer.read_pos = (key_buffer.read_pos + 1) % KEY_BUFFER_SIZE;
    key_buffer.count--;
    return 1;
}

char keyboard_get_key(void) {
    key_event_t event;
    if (!keyboard_get_event(&event)) return 0;
    if (!event.pressed) return 0;
    return (char)(event.keycode & 0xFF);
}



void keyboard_poll(void) {
    // for compatibility reasons
    while(1) {
        __asm__ volatile("hlt");
    }
}

static int keyboard_module_init(void) {
    keyboard_init();
    return 0;
}

static void keyboard_module_fini(void) {
    irq_unregister_handler(1);
}

driver_module keyboard_module = (driver_module) {
    .name = "ps2_keyboard",
    .mount = "/dev/keyboard",
    .version = VERSION_NUM(0, 3, 0, 0),
    .init = keyboard_module_init,
    .fini = keyboard_module_fini,
    .open = NULL, // later for fs
    .read = NULL, // later for fs
    .write = NULL, // later for fs
};

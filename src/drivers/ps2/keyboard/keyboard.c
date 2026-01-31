#include "keyboard.h"
#include "maps.h"
#include <kernel/include/ports.h>
#include <kernel/exceptions/irq.h>
#include <theme/doccr.h>
#include <kernel/graph/theme.h>

static key_buffer_t key_buffer = {0};
static int shift = 0;
static int ctrl = 0;
static int alt = 0;
static int caps = 0;
static int extended = 0; // extended keys (0xE0 prefix)

static void keyboard_put_event(key_event_t event) {
    if (key_buffer.count >= KEY_BUFFER_SIZE) return;

    key_buffer.buffer[key_buffer.write_pos] = event;
    key_buffer.write_pos = (key_buffer.write_pos + 1) % KEY_BUFFER_SIZE;
    key_buffer.count++;
}

static void keyboard_irq_handler(cpu_state_t* state) {
    (void)state;

    const keymap_t *keymap = keymap_get_current();

    if ((inb(0x64) & 1) == 0) return;

    u8 sc = inb(0x60);
    if (sc == 0) return;
    if (sc == 0xE0) {
        extended = 1;
        return;
    }
    if (sc & 0x80) {
        u8 make = sc & 0x7F;
        if (extended) {
            extended = 0;
            return;
        }

        if (make == 0x2A || make == 0x36) shift = 0;
        if (make == 0x1D) ctrl = 0;
        if (make == 0x38) alt = 0;
        return;
    }

    // Handle modifier keys
    if (!extended) {
        if (sc == 0x2A || sc == 0x36) { shift = 1; return; }
        if (sc == 0x1D) { ctrl = 1; return; }
        if (sc == 0x38) { alt = 1; return; }
        if (sc == 0x3A) { caps = !caps; return; }
    }

    // Create key event
    key_event_t event;
    event.pressed = 1;
    event.modifiers = 0;
    if (ctrl) event.modifiers |= KEY_CTRL_MASK;
    if (shift) event.modifiers |= KEY_SHIFT_MASK;
    if (alt) event.modifiers |= KEY_ALT_MASK;

    // Handle extended keys (arrows, etc.)
    if (extended) {
        extended = 0;
        switch (sc) {
            case 0x48: // Up arrow
                event.keycode = KEY_ARROW_UP;
                keyboard_put_event(event);
                return;
            case 0x50: // Down arrow
                event.keycode = KEY_ARROW_DOWN;
                keyboard_put_event(event);
                return;
            case 0x4B: // Left arrow
                event.keycode = KEY_ARROW_LEFT;
                keyboard_put_event(event);
                return;
            case 0x4D: // Right arrow
                event.keycode = KEY_ARROW_RIGHT;
                keyboard_put_event(event);
                return;
            case 0x47: // Home
                event.keycode = KEY_HOME;
                keyboard_put_event(event);
                return;
            case 0x4F: // End
                event.keycode = KEY_END;
                keyboard_put_event(event);
                return;
            case 0x49: // Page Up
                event.keycode = KEY_PAGE_UP;
                keyboard_put_event(event);
                return;
            case 0x51: // Page Down
                event.keycode = KEY_PAGE_DOWN;
                keyboard_put_event(event);
                return;
            case 0x53: // Delete
                event.keycode = KEY_DELETE;
                keyboard_put_event(event);
                return;
            case 0x52: // Insert
                event.keycode = KEY_INSERT;
                keyboard_put_event(event);
                return;
            default:
                return; // Ignore
        }
    }

    // non-extended
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

    // function keys
    if (sc >= 0x3B && sc <= 0x44) { // F1-F10
        event.keycode = KEY_F1 + (sc - 0x3B);
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
    extended = 0;

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
    while(1) {
        __asm__ volatile("hlt");
    }
}

static int keyboard_module_init(void) {
    log("[KEYBOARD]", "Load Keyboard module...\n", d);
    keyboard_init();
    return 0;
}

static void keyboard_module_fini(void) {
    irq_unregister_handler(1);
}

driver_module keyboard_module = (driver_module) {
    .name = "ps2_keyboard",
    .mount = "/emr/drv/keyboard", // because its not the driver not the device
    .version = VERSION_NUM(0, 3, 1, 0),
    .init = keyboard_module_init,
    .fini = keyboard_module_fini,
    .open = NULL,
    .read = NULL,
    .write = NULL,
};

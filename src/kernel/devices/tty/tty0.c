#include "tty0.h"

#include <kernel/module/module.h>
#include <kernel/communication/serial.h>
#include <kernel/console/graph/print.h>
#include <kernel/arch/x86_64/exceptions/irq.h>
#include <drivers/ps2/keyboard/keyboard.h>
#include <theme/doccr.h>
#include <drivers/drivers.h>
#include <types.h>


typedef enum {
    ANSI_NORMAL = 0,
    ANSI_ESC, // got \033
    ANSI_CSI, // got \033[
} tty0_ansi_state_t;

static tty0_ansi_state_t ansi_state = ANSI_NORMAL;
static int ansi_param = 0;
static u32 ansi_fg    = 0xFFFFFFFF; // default white

static u32 tty0_ansi_color(int code) {
    switch (code) {
        case  0: return 0xFFFFFFFF; // reset
        case 30: return 0xFF111111; // black
        case 31: return 0xFFFF5555; // red
        case 32: return 0xFF55FF55; // green
        case 33: return 0xFFFFFF55; // yellow
        case 34: return 0xFF5555FF; // blue
        case 35: return 0xFFFF55FF; // magenta
        case 36: return 0xFF55FFFF; // cyan
        case 37: return 0xFFFFFFFF; // white
        case 90: return 0xFF888888; // bright black/gray
        case 91: return 0xFFFF8888; // bright red
        case 92: return 0xFF88FF88; // bright green
        case 93: return 0xFFFFFF88; // bright yellow
        case 94: return 0xFF8888FF; // bright blue
        case 95: return 0xFFFF88FF; // bright magenta
        case 96: return 0xFF88FFFF; // bright cyan
        case 97: return 0xFFFFFFFF; // bright white
        default: return 0xFFFFFFFF;
    }
}

void tty0_write_char(char c) {
    char tmp[2] = { c, '\0' };
    switch (ansi_state) {
        case ANSI_NORMAL:
            if (c == '\033') {
                ansi_state = ANSI_ESC;
            } else {
                cprintf(tmp, ansi_fg);
            }
            break;

        case ANSI_ESC:
            if (c == '[') {
                ansi_state = ANSI_CSI;
                ansi_param = 0;
            } else {
                cprintf("\033", ansi_fg);
                cprintf(tmp, ansi_fg);
                ansi_state = ANSI_NORMAL;
            }
            break;

        case ANSI_CSI:
            if (c >= '0' && c <= '9') {
                ansi_param = ansi_param * 10 + (c - '0');
            } else if (c == 'm') {
                ansi_fg = tty0_ansi_color(ansi_param);
                ansi_param = 0;
                ansi_state = ANSI_NORMAL;
            } else if (c == ';') {
                ansi_fg = tty0_ansi_color(ansi_param);
                ansi_param = 0;
                // stay in CSI
            } else {
                ansi_param = 0;
                ansi_state = ANSI_NORMAL;
            }
            break;
    }
}


static int tty0_echo_mode = TTY_ECHO; // default == 0

void tty0_set_echo_mode(int mode) { tty0_echo_mode = mode; }
int  tty0_get_echo_mode(void) { return tty0_echo_mode; }

static int tty0_init(void) {
    log("[TTY0]", "init /dev/tty0\n", d);
    return 0;
}

static void tty0_fini(void) {}

static void *tty0_open(const char *path) {
    (void)path;
    return (void *)1; // dummy handle
}

static int tty0_dev_write(void *handle, const void *buf, size_t count) {
    (void)handle;
    const char *p = (const char *)buf;
    for (size_t i = 0; i < count; i++) {
        tty0_write_char(p[i]);
    }
    return (int)count;
}
static int tty0_dev_read(void *handle, void *buf, size_t count) {
    (void)handle;
    if (!buf || count == 0) return 0;

    char *out = (char *)buf;
    size_t i  = 0;

    // lazyopen keyboard device fd once
    static int kbd_fd = -1;
    if (kbd_fd < 0) {
        kbd_fd = fs_open(KEYBOARD0, O_RDONLY);
        if (kbd_fd < 0) {
            printf("[TTY0] read: cannot open " KEYBOARD0 "\n");
            return -1;
        }
    }

    __asm__ volatile("sti");

    while (i < count - 1) {
        key_event_t event;
        int got = 0;

        while (!got) {
            __asm__ volatile("hlt");
            int n = fs_read(kbd_fd, &event, sizeof(key_event_t));
            if (n == (int)sizeof(key_event_t)) got = 1;
        }

        if (!event.pressed) continue;

        char c = (char)(event.keycode & 0xFF);

        if (c == '\n' || c == '\r') {
            // always echo the newline so cursor moves to next line
            tty0_write_char('\n');
            out[i++] = '\n';
            break;
        }

        if (c == '\b') {
            if (i > 0) {
                i--;
                if (tty0_echo_mode != TTY_NOECHO)
                    tty0_write_char('\b');
            }
            continue;
        }

        if (c < 0x20 || c > 0x7E) continue;

        // echo based on current mode
        if (tty0_echo_mode == TTY_ECHO)
            tty0_write_char(c);
        else if (tty0_echo_mode == TTY_MASKECHO)
            tty0_write_char('*');
        // TTY_NOECHO == no output at all

        out[i++] = c;
    }

    __asm__ volatile("cli");

    out[i] = '\0';
    return (int)i;
}

driver_module tty0_module = {
    .name    = TTY0NAME,
    .mount   = TTY0PATH,
    .version = TTY0UNIVERSAL,
    .init    = tty0_init,
    .fini    = tty0_fini,
    .open    = tty0_open,
    .read    = tty0_dev_read,
    .write   = tty0_dev_write,
};
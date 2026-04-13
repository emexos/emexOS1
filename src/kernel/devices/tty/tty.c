#include "tty.h"
#include <kernel/graph/graphics.h>
#include <kernel/graph/theme.h>
#include <kernel/kernel_processes/fm/fm.h>
#include <kernel/kernel_processes/bootscreen/console/console.h>
#include <kernel/kernel_processes/bootscreen/boot.h>
#include <kernel/file_systems/vfs/vfs.h>
#include <drivers/ps2/keyboard/keyboard.h>
#include <memory/main.h>
#include <kernel/communication/serial.h>
#include <theme/doccr.h>

#include <config/system.h>

#define _TTYKBD "/dev/input/keyboard0"

static tty_t tty_table[TTY_COUNT];
static int   tty_active = 0;

static u32 tty_ansi_color(int code)
{
    switch (code)
    {
        case  0: return 0xFFFFFFFF;
        case 30: return 0xFF111111;
        case 31: return 0xFFFF5555;
        case 32: return 0xFF55FF55;
        case 33: return 0xFFFFFF55;
        case 34: return 0xFF5555FF;
        case 35: return 0xFFFF55FF;
        case 36: return 0xFF55FFFF;
        case 37: return 0xFFFFFFFF;
        case 90: return 0xFF888888;
        case 91: return 0xFFFF8888;
        case 92: return 0xFF88FF88;
        case 93: return 0xFFFFFF88;
        case 94: return 0xFF8888FF;
        case 95: return 0xFFFF88FF;
        case 96: return 0xFF88FFFF;
        case 97: return 0xFFFFFFFF;
        default: return 0xFFFFFFFF;
    }
}

void tty_init(void)
{
    memset(tty_table, 0, sizeof(tty_table));
    for (int i = 0; i < TTY_COUNT; i++) {
        tty_table[i].ansi_state = TTY_ANSI_NORMAL;
        tty_table[i].ansi_fg = 0xFFFFFFFF;
        tty_table[i].ansi_bg = 0xFF000000;
        tty_table[i].echo_mode = TTY_ECHO;
        tty_table[i].kbd_fd = -1;
        tty_table[i].valid = 1;
    }
    tty_active = 0;
}

int tty_get_active(void)
{
    return tty_active;
}

void tty_set_active(int id)
{
    if (id < 0 || id >= TTY_COUNT) return;
    if (!tty_table[id].valid) return;

    tty_save_cursor(tty_active);
    tty_active = id;
    tty_restore_cursor(tty_active);
}

tty_t *tty_get(int id)
{
    if (id < 0 || id >= TTY_COUNT) return NULL;
    return &tty_table[id];
}

void tty_save_cursor(int id)
{
    if (id < 0 || id >= TTY_COUNT) return;
    tty_table[id].cursor_x = bs_get_active()->cursor_x;
    tty_table[id].cursor_y = bs_get_active()->cursor_y;
}

void tty_restore_cursor(int id)
{
    if (id < 0 || id >= TTY_COUNT) return;
    bs_get_active()->cursor_x = tty_table[id].cursor_x;
    bs_get_active()->cursor_y = tty_table[id].cursor_y;
}

void tty_set_echo(int id, int mode)
{
    if (id < 0 || id >= TTY_COUNT) return;
    tty_table[id].echo_mode = mode;
}

int tty_get_echo(int id)
{
    if (id < 0 || id >= TTY_COUNT) return 0;
    return tty_table[id].echo_mode;
}

void tty_write_char(int id, char c)
{
    if (id != tty_active) return;

    tty_t *t = tty_get(id);
    if (!t) return;

    char tmp[2] = { c, '\0' };

    switch (t->ansi_state)
    {
        case TTY_ANSI_NORMAL:
            if (c == '\033') {
                t->ansi_state = TTY_ANSI_ESC;
            } else if (c == '\b') {
                u32 char_width  = fm_get_char_width()  * font_scale;
                u32 char_height = fm_get_char_height() * font_scale;
                if (bs_get_active()->cursor_x >= char_width)
                {
                    bs_get_active()->cursor_x -= char_width;
                    #if TTYNOGUI == 1
                    	draw_rect(bs_get_active()->cursor_x, bs_get_active()->cursor_y, char_width, char_height, t->ansi_bg);
                    #endif
                }
            } else {
            	#if TTYNOGUI == 1
                	cprintf(tmp, t->ansi_fg);
                #endif
            }
            break;

        case TTY_ANSI_ESC:
            if (c == '[') {
                t->ansi_state = TTY_ANSI_CSI;
                t->ansi_param = 0;
                t->ansi_private = 0;
            } else {
	            #if TTYNOGUI == 1
	                cprintf("\033", t->ansi_fg);
	                cprintf(tmp,t->ansi_fg);
	            #endif
                t->ansi_state = TTY_ANSI_NORMAL;
            }
            break;

        case TTY_ANSI_CSI:
            if (c >= '0' && c <= '9') {
                t->ansi_param = t->ansi_param * 10 + (c - '0');
            } else if (c == '?') {
                t->ansi_private = 1;
            } else if (c == ';') {
                t->ansi_fg = tty_ansi_color(t->ansi_param);
                t->ansi_param = 0;
            } else if (c == 'm') {
                t->ansi_fg = tty_ansi_color(t->ansi_param);
                t->ansi_param = 0;
                t->ansi_state = TTY_ANSI_NORMAL;
            } else if (c == 'J') {
                if (t->ansi_param == 2) clear(bg());
                t->ansi_param = 0;
                t->ansi_private = 0;
                t->ansi_state = TTY_ANSI_NORMAL;
            } else if (c == 'H') {
                bs_get_active()->cursor_x = 0;
                bs_get_active()->cursor_y = 0;
                t->ansi_param = 0;
                t->ansi_state = TTY_ANSI_NORMAL;
            } else if (c == 'l') {
                t->ansi_param = 0;
                t->ansi_private = 0;
                t->ansi_state = TTY_ANSI_NORMAL;
            } else if (c == 'h') {
                t->ansi_param = 0;
                t->ansi_private = 0;
                t->ansi_state = TTY_ANSI_NORMAL;
            } else {
                t->ansi_param = 0;
                t->ansi_private = 0;
                t->ansi_state = TTY_ANSI_NORMAL;
            }
            break;
    }
}

int tty_dev_read(int id, void *buf, size_t count)
{
    if (id != tty_active) return 0;

    tty_t *t = tty_get(id);
    if (!t || !buf || count == 0) return 0;

    char *out = (char *)buf;
    size_t i  = 0;

    if (t->kbd_fd < 0) {
        t->kbd_fd = fs_open(_TTYKBD, O_RDONLY);
        if (t->kbd_fd < 0) {
            printf("[TTY] read: cannot open " _TTYKBD "\n");
            return -1;
        }
    }

    __asm__ volatile("sti");

    if (t->echo_mode == TTY_RAW)
    {
        while (1)
        {
            key_event_t event;
            int n = fs_read(t->kbd_fd, &event, sizeof(key_event_t));

            __asm__ volatile("hlt");
            if (n != (int)sizeof(key_event_t)) continue;
            if (!event.pressed) continue;

            char c = (char)(event.keycode & 0xFF);
            if (event.keycode == 0x09) c = '\t';
            if (c == '\n' || c == '\r' || c == '\b' || c == '\t' ||
                (c >= 0x20 && c <= 0x7E)) {
                out[0] = (c == '\r') ? '\n' : c;
                __asm__ volatile("cli");
                return 1;
            }
        }
    }

    while (i < count - 1)
    {
        key_event_t event;
        int got = 0;

        while (!got) {
            int n = fs_read(t->kbd_fd, &event, sizeof(key_event_t));
            if (n == (int)sizeof(key_event_t)) { got = 1; break; }
            __asm__ volatile("hlt");
        }

        if (!event.pressed) continue;

        char c = (char)(event.keycode & 0xFF);

        if (c == '\n' || c == '\r') {
            tty_write_char(id, '\n');
            out[i++] = '\n';
            break;
        }
        if (c == '\b') {
            if (i > 0) {
                i--;
                if (t->echo_mode != TTY_NOECHO)
                    tty_write_char(id, '\b');
            }
            continue;
        }
        if (c < 0x20 || c > 0x7E) continue;

        if (t->echo_mode == TTY_ECHO)
            tty_write_char(id, c);
        else if (t->echo_mode == TTY_MASKECHO)
            tty_write_char(id, '*');

        out[i++] = c;
    }

    __asm__ volatile("cli");

    out[i] = '\0';
    return (int)i;
}
#include <kernel/console/console.h>
#include <kernel/file_systems/vfs/vfs.h>
#include <kernel/include/ports.h>

#define MAX_EDIT_BUFFER 4096

static char edit_buffer[MAX_EDIT_BUFFER];
static int edit_len = 0;
static char edit_filename[MAX_PATH_LEN];
static int edit_pos = 0;

// Selection
static int sel_start = -1;
static int sel_end = -1;

static int edit_load(const char *path) {
    int fd = fs_open(path, O_RDONLY);
    if (fd < 0) {
        edit_buffer[0] = '\0';
        edit_len = 0;
        return 0;
    }

    ssize_t bytes = fs_read(fd, edit_buffer, MAX_EDIT_BUFFER - 1);
    fs_close(fd);

    if (bytes < 0) bytes = 0;
    edit_buffer[bytes] = '\0';
    edit_len = bytes;
    return 0;
}

static int edit_save(void) {
    int fd = fs_open(edit_filename, O_CREAT | O_WRONLY);
    if (fd < 0) return -1;

    fs_write(fd, edit_buffer, edit_len);
    fs_close(fd);
    return 0;
}

static void edit_draw(void) {
    clear(CONSOLESCREEN_BG_COLOR);
    banner_draw();

    // Top space
    cursor_x = 0;
    cursor_y = banner_get_height() + 16;

    // Draw content
    for (int i = 0; i < edit_len; i++) {
        u32 bg = CONSOLESCREEN_BG_COLOR;
        u32 fg = GFX_WHITE;

        // Selection
        if (sel_start != -1 && sel_end != -1) {
            int min = (sel_start < sel_end) ? sel_start : sel_end;
            int max = (sel_start < sel_end) ? sel_end : sel_start;
            if (i >= min && i < max) {
                bg = GFX_WHITE;
                fg = GFX_BLACK;
            }
        }

        // Draw char
        if (bg != CONSOLESCREEN_BG_COLOR) {
            draw_rect(cursor_x, cursor_y, 8 * font_scale, 8 * font_scale, bg);
        }

        char c[2] = {edit_buffer[i], '\0'};
        string(c, fg);
    }

    // Bottom bar
    u32 bar_y = get_fb_height() - (8 * font_scale + 4);
    draw_rect(0, bar_y, get_fb_width(), 8 * font_scale + 4, GFX_WHITE);

    cursor_x = 8;
    cursor_y = bar_y + 2;
    string("Ctrl+S: Save | Ctrl+Q: Quit | Arrow keys: Move | Shift+Arrow: Select | Del: Delete", GFX_BLACK);
}

static void delete_sel(void) {
    if (sel_start == -1 || sel_end == -1) return;

    int min = (sel_start < sel_end) ? sel_start : sel_end;
    int max = (sel_start < sel_end) ? sel_end : sel_start;

    for (int i = min; i < edit_len - (max - min); i++) {
        edit_buffer[i] = edit_buffer[i + (max - min)];
    }

    edit_len -= (max - min);
    edit_pos = min;
    sel_start = sel_end = -1;
}

FHDR(cmd_edit) {
    if (!s || *s == '\0') {
        print("usage: edit <file>\n", GFX_RED);
        return;
    }

    str_copy(edit_filename, s);
    edit_load(edit_filename);

    edit_pos = 0;
    sel_start = sel_end = -1;

    int shift = 0;
    int ctrl = 0;

    edit_draw();

    while (1) {
        if ((inb(0x64) & 1) == 0) {
            __asm__ volatile("hlt");
            continue;
        }

        u8 sc = inb(0x60);
        if (sc == 0) continue;

        // Release
        if (sc & 0x80) {
            u8 k = sc & 0x7F;
            if (k == 0x2A || k == 0x36) shift = 0;
            if (k == 0x1D) ctrl = 0;
            continue;
        }

        // Press
        if (sc == 0x2A || sc == 0x36) { shift = 1; continue; }
        if (sc == 0x1D) { ctrl = 1; continue; }

        // Ctrl+S
        if (ctrl && sc == 0x1F) {
            edit_save();
            u32 old_x = cursor_x, old_y = cursor_y;
            draw_rect(get_fb_width()/2 - 50, get_fb_height()/2 - 20, 100, 40, GFX_GREEN);
            cursor_x = get_fb_width()/2 - 30;
            cursor_y = get_fb_height()/2 - 8;
            string("Saved!", GFX_WHITE);
            for (volatile int i = 0; i < 5000000; i++);
            cursor_x = old_x;
            cursor_y = old_y;
            edit_draw();
            continue;
        }

        // Ctrl+Q
        if (ctrl && sc == 0x10) {
            clear(CONSOLESCREEN_BG_COLOR);
            banner_draw();
            console_window_init();
            return;
        }

        // Left
        if (sc == 0x4B) {
            if (edit_pos > 0) {
                edit_pos--;
                if (shift) {
                    if (sel_start == -1) sel_start = edit_pos + 1;
                    sel_end = edit_pos;
                } else {
                    sel_start = sel_end = -1;
                }
                edit_draw();
            }
            continue;
        }

        // Right
        if (sc == 0x4D) {
            if (edit_pos < edit_len) {
                edit_pos++;
                if (shift) {
                    if (sel_start == -1) sel_start = edit_pos - 1;
                    sel_end = edit_pos;
                } else {
                    sel_start = sel_end = -1;
                }
                edit_draw();
            }
            continue;
        }

        // Delete
        if (sc == 0x53) {
            if (sel_start != -1 && sel_end != -1) {
                delete_sel();
            } else if (edit_pos < edit_len) {
                for (int i = edit_pos; i < edit_len - 1; i++) {
                    edit_buffer[i] = edit_buffer[i + 1];
                }
                edit_len--;
            }
            edit_draw();
            continue;
        }

        // Backspace
        if (sc == 0x0E) {
            if (sel_start != -1 && sel_end != -1) {
                delete_sel();
            } else if (edit_pos > 0) {
                for (int i = edit_pos - 1; i < edit_len - 1; i++) {
                    edit_buffer[i] = edit_buffer[i + 1];
                }
                edit_pos--;
                edit_len--;
            }
            edit_draw();
            continue;
        }

        // Enter
        if (sc == 0x1C) {
            if (sel_start != -1) delete_sel();
            if (edit_len < MAX_EDIT_BUFFER - 1) {
                for (int i = edit_len; i > edit_pos; i--) {
                    edit_buffer[i] = edit_buffer[i - 1];
                }
                edit_buffer[edit_pos] = '\n';
                edit_pos++;
                edit_len++;
                edit_draw();
            }
            continue;
        }

        // Normal chars
        char c = 0;
        if (sc >= 0x10 && sc <= 0x19) c = "qwertyuiop"[sc - 0x10];
        else if (sc >= 0x1E && sc <= 0x26) c = "asdfghjkl"[sc - 0x1E];
        else if (sc >= 0x2C && sc <= 0x32) c = "zxcvbnm"[sc - 0x2C];
        else if (sc == 0x39) c = ' ';
        else if (sc == 0x02) c = shift ? '!' : '1';
        else if (sc == 0x03) c = shift ? '@' : '2';
        else if (sc == 0x04) c = shift ? '#' : '3';
        else if (sc == 0x05) c = shift ? '$' : '4';
        else if (sc == 0x06) c = shift ? '%' : '5';
        else if (sc == 0x07) c = shift ? '^' : '6';
        else if (sc == 0x08) c = shift ? '&' : '7';
        else if (sc == 0x09) c = shift ? '*' : '8';
        else if (sc == 0x0A) c = shift ? '(' : '9';
        else if (sc == 0x0B) c = shift ? ')' : '0';

        if (c) {
            if (sel_start != -1) delete_sel();
            if (edit_len < MAX_EDIT_BUFFER - 1) {
                for (int i = edit_len; i > edit_pos; i--) {
                    edit_buffer[i] = edit_buffer[i - 1];
                }
                edit_buffer[edit_pos] = shift ? ((c >= 'a' && c <= 'z') ? c - 32 : c) : c;
                edit_pos++;
                edit_len++;
                edit_draw();
            }
        }
    }
}

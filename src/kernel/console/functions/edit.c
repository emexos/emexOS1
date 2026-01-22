#include <kernel/console/console.h>
#include <kernel/file_systems/vfs/vfs.h>
#include <drivers/ps2/keyboard/keyboard.h>

#define MAX_EDIT_SIZE 4096
#define LINES_MAX 200
#define LINE_MAX_LEN 256

static char lines[LINES_MAX][LINE_MAX_LEN];
static int line_count = 0;
static int cursor_x_pos = 0;
static int cursor_y_pos = 0;
static int scroll_offset = 0;
static int modified = 0;
static char current_file[MAX_PATH_LEN];
static int redraw_needed = 1;

static void clear_lines(void) {
    for (int i = 0; i < LINES_MAX; i++) {
        lines[i][0] = '\0';
    }
    line_count = 1;
    cursor_x_pos = 0;
    cursor_y_pos = 0;
    scroll_offset = 0;
}

static int load_file(const char *path) {
    int fd = fs_open(path, O_RDONLY);
    if (fd < 0) {
        clear_lines();
        return 0;
    }

    char buffer[MAX_EDIT_SIZE];
    ssize_t bytes = fs_read(fd, buffer, MAX_EDIT_SIZE - 1);
    fs_close(fd);

    if (bytes <= 0) {
        clear_lines();
        return 0;
    }

    buffer[bytes] = '\0';

    clear_lines();
    line_count = 0;
    int col = 0;

    for (int i = 0; i < bytes; i++) {
        if (buffer[i] == '\n') {
            lines[line_count][col] = '\0';
            line_count++;
            col = 0;
            if (line_count >= LINES_MAX) break;
        } else if (buffer[i] != '\r') {
            if (col < LINE_MAX_LEN - 1) {
                lines[line_count][col++] = buffer[i];
            }
        }
    }

    if (col > 0 || line_count == 0) {
        lines[line_count][col] = '\0';
        line_count++;
    }

    if (line_count == 0) {
        line_count = 1;
        lines[0][0] = '\0';
    }

    return 0;
}

static int save_file(void) {
    // Öffne Datei mit Truncate um alten Inhalt zu überschreiben
    int fd = fs_open(current_file, O_CREAT | O_WRONLY);
    if (fd < 0) {
        return -1;
    }

    // Schreibe alle Zeilen
    for (int i = 0; i < line_count; i++) {
        int len = str_len(lines[i]);
        if (len > 0) {
            ssize_t written = fs_write(fd, lines[i], len);
            if (written < 0) {
                fs_close(fd);
                return -1;
            }
        }

        // Füge Newline hinzu außer bei letzter Zeile
        if (i < line_count - 1) {
            fs_write(fd, "\n", 1);
        }
    }

    fs_close(fd);
    modified = 0;
    return 0;
}

static void draw_cursor_only(void) {
    u32 start_y = banner_get_height() + (8 * font_scale) + 8; // Banner + Border
    u32 line_height = 8 * font_scale + 2;

    int display_line = cursor_y_pos - scroll_offset;
    if (display_line < 0) return;

    u32 y = start_y + display_line * line_height;

    // Berechne X-Position (mit Zeilennummer)
    char num[8];
    str_copy(num, "");
    str_append_uint(num, cursor_y_pos + 1);
    str_append(num, " ");
    u32 num_width = str_len(num) * (8 * font_scale);

    u32 x = 8 + num_width + cursor_x_pos * (8 * font_scale);

    // Zeichne Cursor
    u32 char_width = 8 * font_scale;
    draw_rect(x, y, char_width, 8 * font_scale, GFX_YELLOW);

    // Zeige Zeichen unter Cursor oder Leerzeichen
    if (cursor_x_pos < str_len(lines[cursor_y_pos])) {
        char c[2] = {lines[cursor_y_pos][cursor_x_pos], '\0'};
        u32 old_x = cursor_x;
        u32 old_y = cursor_y;
        cursor_x = x;
        cursor_y = y;
        string(c, GFX_BLACK);
        cursor_x = old_x;
        cursor_y = old_y;
    }
}

static void Edraw_line(int line_idx, int display_idx) {
    u32 start_y = banner_get_height() + (8 * font_scale) + 8;
    u32 line_height = 8 * font_scale + 2;

    cursor_x = 8;
    cursor_y = start_y + display_idx * line_height;

    // Lösche alte Zeile
    draw_rect(0, cursor_y, get_fb_width(), line_height, CONSOLESCREEN_BG_COLOR);

    cursor_x = 8;

    // Zeilennummer
    char num[8];
    str_copy(num, "");
    str_append_uint(num, line_idx + 1);
    str_append(num, " ");
    string(num, GFX_GRAY_50);

    // Zeileninhalt
    for (int j = 0; lines[line_idx][j] != '\0'; j++) {
        char c[2] = {lines[line_idx][j], '\0'};
        string(c, GFX_WHITE);
    }
}

static void draw_editor(void) {
    clear(CONSOLESCREEN_BG_COLOR);
    banner_draw();

    // Zeichne obere Border-Linie (unterhalb Banner)
    u32 border_y = banner_get_height();
    draw_rect(0, border_y, get_fb_width(), 8 * font_scale, CONSOLESCREEN_BG_COLOR);

    u32 start_y = border_y + (8 * font_scale) + 8;
    u32 max_y = get_fb_height() - (16 * font_scale);
    u32 line_height = 8 * font_scale + 2;
    u32 lines_visible = (max_y - start_y) / line_height;

    // Zeichne alle sichtbaren Zeilen
    for (u32 i = 0; i < lines_visible && (scroll_offset + i) < line_count; i++) {
        Edraw_line(scroll_offset + i, i);
    }

    // Zeichne Cursor
    draw_cursor_only();

    // Statuszeile
    u32 bar_y = get_fb_height() - (12 * font_scale);
    draw_rect(0, bar_y, get_fb_width(), 12 * font_scale, GFX_WHITE);

    cursor_x = 8;
    cursor_y = bar_y + 2;

    string(current_file, GFX_BLACK);

    if (modified) {
        string(" [*]", GFX_BLACK);
    }

    char info[64];
    str_copy(info, " | Ln:");
    str_append_uint(info, cursor_y_pos + 1);
    str_append(info, "/");
    str_append_uint(info, line_count);
    str_append(info, " Col:");
    str_append_uint(info, cursor_x_pos + 1);
    string(info, GFX_BLACK);

    cursor_x = get_fb_width() - 250;
    string("Ctrl+S:Save | Ctrl+Q:Quit", GFX_BLACK);
}

static void update_status_bar(void) {
    u32 bar_y = get_fb_height() - (12 * font_scale);
    draw_rect(0, bar_y, get_fb_width(), 12 * font_scale, GFX_WHITE);

    cursor_x = 8;
    cursor_y = bar_y + 2;

    string(current_file, GFX_BLACK);

    if (modified) {
        string(" [*]", GFX_BLACK);
    }

    char info[64];
    str_copy(info, " | Ln:");
    str_append_uint(info, cursor_y_pos + 1);
    str_append(info, "/");
    str_append_uint(info, line_count);
    str_append(info, " Col:");
    str_append_uint(info, cursor_x_pos + 1);
    string(info, GFX_BLACK);

    cursor_x = get_fb_width() - 250;
    string("Ctrl+S:Save | Ctrl+Q:Quit", GFX_BLACK);
}

static void redraw_current_line(void) {
    int display_idx = cursor_y_pos - scroll_offset;
    if (display_idx >= 0) {
        Edraw_line(cursor_y_pos, display_idx);
    }
}

static void insert_char(char c) {
    int len = str_len(lines[cursor_y_pos]);

    if (len >= LINE_MAX_LEN - 1) return;

    for (int i = len; i > cursor_x_pos; i--) {
        lines[cursor_y_pos][i] = lines[cursor_y_pos][i - 1];
    }

    lines[cursor_y_pos][cursor_x_pos] = c;
    lines[cursor_y_pos][len + 1] = '\0';

    cursor_x_pos++;
    modified = 1;

    // Nur aktuelle Zeile neu zeichnen
    redraw_current_line();
    draw_cursor_only();
    //update_status_bar();
}

static void delete_char(void) {
    int len = str_len(lines[cursor_y_pos]);

    if (cursor_x_pos == 0) {
        if (cursor_y_pos > 0) {
            int prev_len = str_len(lines[cursor_y_pos - 1]);

            str_append(lines[cursor_y_pos - 1], lines[cursor_y_pos]);

            for (int i = cursor_y_pos; i < line_count - 1; i++) {
                str_copy(lines[i], lines[i + 1]);
            }

            line_count--;
            cursor_y_pos--;
            cursor_x_pos = prev_len;
            modified = 1;
            redraw_needed = 1; // Komplettes Redraw nötig
        }
        return;
    }

    if (cursor_x_pos > 0) {
        for (int i = cursor_x_pos - 1; i < len; i++) {
            lines[cursor_y_pos][i] = lines[cursor_y_pos][i + 1];
        }
        cursor_x_pos--;
        modified = 1;

        redraw_current_line();
        draw_cursor_only();
        //update_status_bar();
    }
}

static void insert_newline(void) {
    if (line_count >= LINES_MAX) return;

    char rest[LINE_MAX_LEN];
    str_copy(rest, &lines[cursor_y_pos][cursor_x_pos]);
    lines[cursor_y_pos][cursor_x_pos] = '\0';

    for (int i = line_count; i > cursor_y_pos + 1; i--) {
        str_copy(lines[i], lines[i - 1]);
    }

    str_copy(lines[cursor_y_pos + 1], rest);
    line_count++;
    cursor_y_pos++;
    cursor_x_pos = 0;
    modified = 1;
    redraw_needed = 1; // Komplettes Redraw nötig
}

FHDR(cmd_edit) {
    if (!s || *s == '\0') {
        print("usage: edit <file>\n", GFX_RED);
        return;
    }

    str_copy(current_file, s);
    load_file(current_file);

    cursor_x_pos = 0;
    cursor_y_pos = 0;
    scroll_offset = 0;
    modified = 0;
    redraw_needed = 1;

    draw_editor();

    while (1) {
        if (!keyboard_has_key()) {
            __asm__ volatile("hlt");
            continue;
        }

        key_event_t event;
        if (!keyboard_get_event(&event)) continue;
        if (!event.pressed) continue;

        // Lösche alten Cursor vor Aktion
        if (!redraw_needed) {
            redraw_current_line();
        }

        // Ctrl+S - Save
        if ((event.modifiers & KEY_CTRL_MASK) && event.keycode == 's') {
            int result = save_file();

            u32 old_x = cursor_x, old_y = cursor_y;
            u32 msg_w = 120, msg_h = 40;
            draw_rect(get_fb_width()/2 - msg_w/2, get_fb_height()/2 - msg_h/2,
                     msg_w, msg_h, result == 0 ? GFX_GREEN : GFX_RED);
            cursor_x = get_fb_width()/2 - 30;
            cursor_y = get_fb_height()/2 - 8;
            string(result == 0 ? "Saved!" : "Error!", GFX_WHITE);

            for (volatile int i = 0; i < 5000000; i++);

            cursor_x = old_x;
            cursor_y = old_y;
            redraw_needed = 1;
        }

        // Ctrl+Q - Quit
        else if ((event.modifiers & KEY_CTRL_MASK) && event.keycode == 'q') {
            clear(CONSOLESCREEN_BG_COLOR);
            banner_draw();
            console_window_init();
            return;
        }

        // Backspace
        else if ((char)(event.keycode & 0xFF) == '\b') {
            delete_char();
        }

        // Enter
        else if ((char)(event.keycode & 0xFF) == '\n' || (char)(event.keycode & 0xFF) == '\r') {
            insert_newline();
        }

        // Printable characters
        else {
            char c = (char)(event.keycode & 0xFF);
            if (c >= 32 && c <= 126) {
                insert_char(c);
            }
        }

        // Komplettes Redraw wenn nötig
        if (redraw_needed) {
            draw_editor();
            redraw_needed = 0;
        }
    }
}

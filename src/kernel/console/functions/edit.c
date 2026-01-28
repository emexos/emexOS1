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
static int old_cursor_x_pos = 0;  // Track old cursor position
static int old_cursor_y_pos = 0;
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
    old_cursor_x_pos = 0;
    old_cursor_y_pos = 0;
    scroll_offset = 0;
}

static int load_file(const char *path) {
    // Erstelle vollständigen Pfad mit cwd wenn relativer Pfad
    char full_path[MAX_PATH_LEN];
    if (path[0] == '/') {
        str_copy(full_path, path);
    } else {
        extern char cwd[];
        str_copy(full_path, cwd);
        if (cwd[str_len(cwd) - 1] != '/') {
            str_append(full_path, "/");
        }
        str_append(full_path, path);
    }

    printf("[EDIT] Loading from full path: %s\n", full_path);

    int fd = fs_open(full_path, O_RDONLY);
    if (fd < 0) {
        // File doesn't exist, start with empty file
        clear_lines();
        printf("[EDIT] New file: %s (fd=%d)\n", full_path, fd);
        return 0;
    }

    printf("[EDIT] File opened successfully, fd=%d\n", fd);

    char buffer[MAX_EDIT_SIZE];
    ssize_t bytes = fs_read(fd, buffer, MAX_EDIT_SIZE - 1);

    printf("[EDIT] Read returned %d bytes\n", (int)bytes);

    fs_close(fd);

    if (bytes <= 0) {
        clear_lines();
        printf("[EDIT] No data read, starting with empty file\n");
        return 0;
    }

    buffer[bytes] = '\0';

    printf("[EDIT] First 50 chars: '");
    for (int i = 0; i < bytes && i < 50; i++) {
        if (buffer[i] >= 32 && buffer[i] <= 126) {
            printf("%c", buffer[i]);
        } else if (buffer[i] == '\n') {
            printf("\\n");
        } else {
            printf(".");
        }
    }
    printf("'\n");

    clear_lines();
    line_count = 0;
    int col = 0;

    for (int i = 0; i < bytes; i++) {
        if (buffer[i] == '\n') {
            lines[line_count][col] = '\0';
            printf("[EDIT] Line %d complete: '%s' (%d chars)\n", line_count, lines[line_count], col);
            line_count++;
            col = 0;
            if (line_count >= LINES_MAX) break;
        } else if (buffer[i] != '\r') {
            if (col < LINE_MAX_LEN - 1) {
                lines[line_count][col++] = buffer[i];
            }
        }
    }

    // Add last line if there's content or if file was empty
    if (col > 0 || line_count == 0) {
        lines[line_count][col] = '\0';
        printf("[EDIT] Last line: '%s' (%d chars)\n", lines[line_count], col);
        line_count++;
    }

    if (line_count == 0) {
        line_count = 1;
        lines[0][0] = '\0';
    }

    printf("[EDIT] Parsed into %d lines total\n", line_count);
    for (int i = 0; i < line_count && i < 10; i++) {
        printf("[EDIT] Line %d: '%s'\n", i, lines[i]);
    }

    return 0;
}

static int save_file(void) {
    printf("[EDIT] Saving to: %s\n", current_file);

    // Erstelle vollständigen Pfad mit cwd wenn relativer Pfad
    char full_path[MAX_PATH_LEN];
    if (current_file[0] == '/') {
        str_copy(full_path, current_file);
    } else {
        extern char cwd[];
        str_copy(full_path, cwd);
        if (cwd[str_len(cwd) - 1] != '/') {
            str_append(full_path, "/");
        }
        str_append(full_path, current_file);
    }

    printf("[EDIT] Full path: %s\n", full_path);

    // First, try to open the file - this will tell us if it exists
    int test_fd = fs_open(full_path, O_RDONLY);
    int file_exists = (test_fd >= 0);
    if (file_exists) {
        fs_close(test_fd);
        printf("[EDIT] File exists, will overwrite\n");
    }

    // Open for writing - O_CREAT will create if needed
    int fd = fs_open(full_path, O_CREAT | O_WRONLY);

    if (fd < 0) {
        printf("[EDIT] Failed to open file (fd=%d)\n", fd);
        return -1;
    }

    // Calculate total size needed
    u64 total_size = 0;
    for (int i = 0; i < line_count; i++) {
        total_size += str_len(lines[i]);
        if (i < line_count - 1) {
            total_size++; // for newline
        }
    }

    printf("[EDIT] Writing %d bytes total\n", (int)total_size);

    // Schreibe alle Zeilen
    for (int i = 0; i < line_count; i++) {
        int len = str_len(lines[i]);

        if (len > 0) {
            ssize_t written = fs_write(fd, lines[i], len);
            printf("[EDIT] Line %d: wrote %d/%d bytes\n", i, (int)written, len);

            if (written < 0 || written != len) {
                printf("[EDIT] Write failed on line %d\n", i);
                fs_close(fd);
                return -1;
            }
        }

        // Newline außer bei letzter Zeile
        if (i < line_count - 1) {
            ssize_t written = fs_write(fd, "\n", 1);
            if (written < 0) {
                printf("[EDIT] Failed to write newline\n");
                fs_close(fd);
                return -1;
            }
        }
    }

    printf("[EDIT] Closing file\n");
    fs_close(fd);
    modified = 0;
    printf("[EDIT] Save successful\n");
    return 0;
}

static void draw_editor_line(int line_idx, int display_idx) {
    u32 start_y = banner_get_height() + (8 * font_scale) + 8;
    u32 line_height = 8 * font_scale + 2;

    // Speichere alte Cursor-Position
    u32 old_cursor_x = cursor_x;
    u32 old_cursor_y = cursor_y;

    u32 line_y = start_y + display_idx * line_height;

    // Lösche die Zeile komplett
    draw_rect(0, line_y, get_fb_width(), line_height, CONSOLESCREEN_BG_COLOR);

    // Setze temporäre Cursor-Position
    cursor_x = 8;
    cursor_y = line_y;

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

    // Stelle alte Cursor-Position wieder her
    cursor_x = old_cursor_x;
    cursor_y = old_cursor_y;
}

static void clear_cursor_at(int x_pos, int y_pos) {
    if (y_pos < scroll_offset) return;

    u32 start_y = banner_get_height() + (8 * font_scale) + 8;
    u32 line_height = 8 * font_scale + 2;
    int display_line = y_pos - scroll_offset;

    u32 y = start_y + display_line * line_height;

    char num[8];
    str_copy(num, "");
    str_append_uint(num, y_pos + 1);
    str_append(num, " ");
    u32 num_width = str_len(num) * (8 * font_scale);

    u32 x = 8 + num_width + x_pos * (8 * font_scale);
    u32 char_width = 8 * font_scale;

    // Lösche den alten Cursor
    draw_rect(x, y, char_width, 8 * font_scale, CONSOLESCREEN_BG_COLOR);

    // Zeichne das Zeichen an dieser Position neu (falls vorhanden)
    if (x_pos < str_len(lines[y_pos])) {
        char c[2] = {lines[y_pos][x_pos], '\0'};
        u32 old_x = cursor_x;
        u32 old_y = cursor_y;
        cursor_x = x;
        cursor_y = y;
        string(c, GFX_WHITE);
        cursor_x = old_x;
        cursor_y = old_y;
    }
}

static void draw_cursor_only(void) {
    u32 start_y = banner_get_height() + (8 * font_scale) + 8;
    u32 line_height = 8 * font_scale + 2;

    int display_line = cursor_y_pos - scroll_offset;
    if (display_line < 0) return;

    u32 y = start_y + display_line * line_height;

    char num[8];
    str_copy(num, "");
    str_append_uint(num, cursor_y_pos + 1);
    str_append(num, " ");
    u32 num_width = str_len(num) * (8 * font_scale);

    u32 x = 8 + num_width + cursor_x_pos * (8 * font_scale);

    u32 char_width = 8 * font_scale;
    draw_rect(x, y, char_width, 8 * font_scale, GFX_YELLOW);

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

static void draw_editor(void) {
    clear(CONSOLESCREEN_BG_COLOR);
    banner_draw();

    u32 border_y = banner_get_height();
    draw_rect(0, border_y, get_fb_width(), 8 * font_scale, CONSOLESCREEN_BG_COLOR);

    u32 start_y = border_y + (8 * font_scale) + 8;
    u32 max_y = get_fb_height() - (16 * font_scale);
    u32 line_height = 8 * font_scale + 2;
    u32 lines_visible = (max_y - start_y) / line_height;

    for (u32 i = 0; i < lines_visible && (scroll_offset + i) < (u32)line_count; i++) {
        draw_editor_line(scroll_offset + i, i);
    }

    draw_cursor_only();

    u32 bar_y = get_fb_height() - (12 * font_scale);
    draw_rect(0, bar_y, get_fb_width(), 12 * font_scale, GFX_WHITE);

    u32 old_cursor_x = cursor_x;
    u32 old_cursor_y = cursor_y;

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

    cursor_x = old_cursor_x;
    cursor_y = old_cursor_y;

    // Update old cursor position
    old_cursor_x_pos = cursor_x_pos;
    old_cursor_y_pos = cursor_y_pos;
}

static void redraw_current_line(void) {
    int display_idx = cursor_y_pos - scroll_offset;
    if (display_idx >= 0) {
        draw_editor_line(cursor_y_pos, display_idx);
    }
}

static void handle_arrow_up(void) {
    if (cursor_y_pos > 0) {
        cursor_y_pos--;

        int line_len = str_len(lines[cursor_y_pos]);
        if (cursor_x_pos > line_len) {
            cursor_x_pos = line_len;
        }

        if (cursor_y_pos < scroll_offset) {
            scroll_offset = cursor_y_pos;
            redraw_needed = 1;
        }
    }
}

static void handle_arrow_down(void) {
    if (cursor_y_pos < line_count - 1) {
        cursor_y_pos++;

        int line_len = str_len(lines[cursor_y_pos]);
        if (cursor_x_pos > line_len) {
            cursor_x_pos = line_len;
        }

        u32 border_y = banner_get_height();
        u32 start_y = border_y + (8 * font_scale) + 8;
        u32 max_y = get_fb_height() - (16 * font_scale);
        u32 line_height = 8 * font_scale + 2;
        u32 lines_visible = (max_y - start_y) / line_height;

        if (cursor_y_pos >= scroll_offset + (int)lines_visible) {
            scroll_offset = cursor_y_pos - lines_visible + 1;
            redraw_needed = 1;
        }
    }
}

static void handle_arrow_left(void) {
    if (cursor_x_pos > 0) {
        cursor_x_pos--;
    } else if (cursor_y_pos > 0) {
        cursor_y_pos--;
        cursor_x_pos = str_len(lines[cursor_y_pos]);

        if (cursor_y_pos < scroll_offset) {
            scroll_offset = cursor_y_pos;
            redraw_needed = 1;
        }
    }
}

static void handle_arrow_right(void) {
    int line_len = str_len(lines[cursor_y_pos]);
    if (cursor_x_pos < line_len) {
        cursor_x_pos++;
    } else if (cursor_y_pos < line_count - 1) {
        cursor_y_pos++;
        cursor_x_pos = 0;

        u32 border_y = banner_get_height();
        u32 start_y = border_y + (8 * font_scale) + 8;
        u32 max_y = get_fb_height() - (16 * font_scale);
        u32 line_height = 8 * font_scale + 2;
        u32 lines_visible = (max_y - start_y) / line_height;

        if (cursor_y_pos >= scroll_offset + (int)lines_visible) {
            scroll_offset = cursor_y_pos - lines_visible + 1;
            redraw_needed = 1;
        }
    }
}

static void handle_home(void) {
    cursor_x_pos = 0;
}

static void handle_end(void) {
    cursor_x_pos = str_len(lines[cursor_y_pos]);
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

    // Zeichne nur die aktuelle Zeile neu
    redraw_current_line();
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
            redraw_needed = 1;
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
    }
}

static void delete_char_forward(void) {
    int len = str_len(lines[cursor_y_pos]);

    if (cursor_x_pos < len) {
        for (int i = cursor_x_pos; i < len; i++) {
            lines[cursor_y_pos][i] = lines[cursor_y_pos][i + 1];
        }
        modified = 1;

        redraw_current_line();
    } else if (cursor_y_pos < line_count - 1) {
        str_append(lines[cursor_y_pos], lines[cursor_y_pos + 1]);

        for (int i = cursor_y_pos + 1; i < line_count - 1; i++) {
            str_copy(lines[i], lines[i + 1]);
        }

        line_count--;
        modified = 1;
        redraw_needed = 1;
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
    redraw_needed = 1;
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
    old_cursor_x_pos = 0;
    old_cursor_y_pos = 0;
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

        // Lösche alten Cursor explizit vor jeder Änderung
        if (!redraw_needed) {
            clear_cursor_at(old_cursor_x_pos, old_cursor_y_pos);
        }

        // Verwende die Keycodes aus keyboard.h
        u32 keycode = event.keycode;

        // Pfeiltasten mit den definierten Konstanten aus keyboard.h
        if (keycode == KEY_ARROW_UP) {
            handle_arrow_up();
        }
        else if (keycode == KEY_ARROW_DOWN) {
            handle_arrow_down();
        }
        else if (keycode == KEY_ARROW_LEFT) {
            handle_arrow_left();
        }
        else if (keycode == KEY_ARROW_RIGHT) {
            handle_arrow_right();
        }
        else if (keycode == KEY_HOME) {
            handle_home();
        }
        else if (keycode == KEY_END) {
            handle_end();
        }
        else if (keycode == KEY_DELETE) {
            delete_char_forward();
        }
        // Ctrl+S - Save
        else if ((event.modifiers & KEY_CTRL_MASK) && (keycode == 's' || keycode == 'S')) {
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
        else if ((event.modifiers & KEY_CTRL_MASK) && (keycode == 'q' || keycode == 'Q')) {
            clear(CONSOLESCREEN_BG_COLOR);
            banner_draw();
            console_window_init();
            return;
        }
        // Backspace
        else if (keycode == '\b') {
            delete_char();
        }
        // Enter
        else if (keycode == '\n' || keycode == '\r') {
            insert_newline();
        }
        // Printable characters (ASCII 32-126)
        else if (keycode >= 32 && keycode <= 126) {
            char c = (char)keycode;
            insert_char(c);
        }

        // Redraw wenn nötig
        if (redraw_needed) {
            draw_editor();
            redraw_needed = 0;
        } else {
            draw_cursor_only();
        }

        // Update old cursor position tracking
        old_cursor_x_pos = cursor_x_pos;
        old_cursor_y_pos = cursor_y_pos;
    }
}

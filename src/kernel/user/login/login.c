/*#include "login.h"
#include <drivers/ps2/keyboard/keyboard.h>
#include <drivers/cmos/cmos.h>
#include <kernel/graph/graphics.h>
#include <string/string.h>
#include <memory/main.h>
#include <config/user_config.h>

// YEAH ik... this shouldn't be part of the console
// but i don't even switched to usermode yet so its not important...

static char password_buffer[MAX_PASSWORD_LEN];
static int password_pos = 0;

static void draw_login_screen(const char *message, u32 msg_color) {
    clear(CONSOLESCREEN_BG_COLOR);

    u32 screen_w = get_fb_width();
    u32 center_x = screen_w / 2;
    u32 y_pos = 100;

    font_scale = 4;
    const char *username = uci_get_user_name();
    u32 username_width = str_len(username) * (8 * font_scale);

    cursor_x = center_x - (username_width / 2);
    cursor_y = y_pos;
    string(username, GFX_WHITE);

    y_pos += 80;
    font_scale = 2;

    u32 box_width = 450;
    u32 box_height = 30;
    u32 box_x = center_x - (box_width / 2);
    u32 box_y = y_pos;

    draw_rect(box_x - 2, box_y - 2, box_width + 4, box_height + 4, GFX_GRAY_20);
    draw_rect(box_x, box_y, box_width, box_height, GFX_GRAY_20);

    cursor_x = box_x + 10;
    cursor_y = box_y + 8;

    for (int i = 0; i < password_pos; i++) {
        putchar('*', GFX_WHITE);
    }

    u32 cursor_bar_x = cursor_x;
    draw_rect(cursor_bar_x, cursor_y, 2, 8 * font_scale, GFX_WHITE);

    if (message && *message) {
        y_pos += 70;
        u32 msg_width = str_len(message) * (8 * font_scale);
        cursor_x = center_x - (msg_width / 2);
        cursor_y = y_pos;
        string(message, msg_color);
    }

    font_scale = 2;
}

static int verify_password(void) {
    password_buffer[password_pos] = '\0';
    const char *correct_pwd = uci_get_password();
    return str_equals(password_buffer, correct_pwd);
}

int login_authenticate(void) {
    int attempts = 0;
    password_pos = 0;
    memset(password_buffer, 0, MAX_PASSWORD_LEN);

    draw_login_screen("", GFX_WHITE);

    while (attempts < MAX_ATTEMPTS) {

        if (!keyboard_has_key()) {
            __asm__ volatile("hlt");
            continue;
        }

        key_event_t event;
        if (!keyboard_get_event(&event)) continue;
        if (!event.pressed) continue;

        char c = (char)(event.keycode & 0xFF);

        if (c == '\n') {
            if (password_pos == 0) {
                draw_login_screen("", GFX_WHITE);
                continue;
            }

            if (verify_password()) {
                clear(CONSOLESCREEN_BG_COLOR);
                return 1;
            } else {
                attempts++;
                password_pos = 0;
                memset(password_buffer, 0, MAX_PASSWORD_LEN);

                if (attempts >= MAX_ATTEMPTS) {
                    draw_login_screen("your computer is now locked", GFX_RED);

                    for (volatile int i = 0; i < 30000000; i++) {
                        __asm__ volatile("nop");
                    }

                    while (1) {
                        __asm__ volatile("cli; hlt");
                    }
                } else {
                    char msg[64];
                    str_copy(msg, "wrong: ");
                    str_append_uint(msg, MAX_ATTEMPTS - attempts);
                    str_append(msg, " left");
                    draw_login_screen(msg, GFX_WHITE);
                }
            }
        }
        else if (c == '\b') {
            if (password_pos > 0) {
                password_pos--;
                password_buffer[password_pos] = '\0';
                draw_login_screen("", GFX_WHITE);
            }
        }
        else if (c >= 32 && c <= 126) {
            if (password_pos < MAX_PASSWORD_LEN - 1) {
                password_buffer[password_pos++] = c;
                draw_login_screen("", GFX_WHITE);
            }
        }
    }

    return 0;
}
*/

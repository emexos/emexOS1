#include "dos.h"
#include "uno.h"
#include <klib/graphics/graphics.h>

void console_window_init(void) {
    cursor_x = CONSOLE_PADDING_X;
    cursor_y = banner_get_height();
}

void console_window_clear(u32 color)
{
    u32 fb_w = get_fb_width();
    u32 fb_h = get_fb_height();
    u32 banner_h = banner_get_height();

    // clear below banner
    draw_rect(0, banner_h, fb_w, fb_h - banner_h, color);
    //TODO:
    // implement real dirty rectangles/ screen clear

    // redraw banner
    banner_draw();

    // reset cursor to console start
    cursor_x = CONSOLE_PADDING_X;
    cursor_y = banner_h;
}

u32 console_window_get_start_y(void) {
    return banner_get_height();
}

u32 console_window_get_max_y(void) {
    return get_fb_height();
}

void console_window_check_scroll(void)
{
    u32 char_height = 8 * font_scale;
    u32 fb_h = get_fb_height();
    u32 banner_h = banner_get_height();

    // check if cursor is at bottom
    if (cursor_y + char_height >= fb_h) {
        u32 line_height = char_height + 2 * font_scale;

        // scroll only console area
        u32 fb_w = get_fb_width();
        u32 pitch_dwords = get_fb_pitch() / 4;
        u32 *fb = get_framebuffer();

        // move content up
        for (u32 y = banner_h + line_height; y < fb_h; y++) {
            for (u32 x = 0; x < fb_w; x++) {
                fb[(y - line_height) * pitch_dwords + x] = fb[y * pitch_dwords + x];
            }
        }

        // cls top line
        for (u32 y = fb_h - line_height; y < fb_h; y++) {
            for (u32 x = 0; x < fb_w; x++) {
                fb[y * pitch_dwords + x] = CONSOLESCREEN_BG_COLOR;
            }
        }

        //adjust cursor
        cursor_y = fb_h - char_height - 10;
        if (cursor_y < banner_h) {
            cursor_y = banner_h;
        }
    }
}

void console_window_update_layout(void)
{
    //after fsize change
    // adjust cursor
    u32 banner_h = banner_get_height();

    if (cursor_y < banner_h) {
        cursor_y = banner_h;
    }
}

#include "uno.h"
#include <kernel/graph/graphics.h>
#include <kernel/graph/fm.h>
#include <drivers/cmos/cmos.h>
#include <string/string.h>
#include <config/system.h>
#include <kernel/arch/x86_64/exceptions/timer.h>

#include <kernel/console/console.h>

static u32 banner_y = 0;
static u32 banner_y_s = BANNER_Y_SPACING;
static u8 last_second = 0;
static u8 needs_update = 1;
static u32 current_banner_height = 0;//BANNER_HEIGHT;

// forward declaration
static void banner_timer_callback(void);

void banner_init(void)
{
    banner_y = 0;
    banner_y_s = BANNER_Y_SPACING;
    last_second = 0;
    needs_update = 1;
    //current_banner_height = BANNER_HEIGHT;

    u32 char_height = fm_get_char_height() * font_scale;
    current_banner_height = char_height + (banner_y_s * 2);

    timer_register_callback(banner_timer_callback);
    banner_draw();
}

void banner_draw(void)
{
    u32 fb_w = get_fb_width();

    // based on font scale
    u32 char_height = fm_get_char_height() * font_scale;
    u32 o_cursor_x = cursor_x;
    u32 o_cursor_y = cursor_y;
    u32 o_scale = font_scale;
    current_banner_height = char_height + (banner_y_s * 2);

    // draw background (in future moved to the wm)
    draw_rect(0, banner_y, fb_w, current_banner_height, BANNER_BG_COLOR);
    draw_rect(0, banner_y + current_banner_height - font_scale, fb_w, font_scale, BANNER_BORDER_COLOR);

    // use current screen_scale for banner

    cursor_x = 4;
    cursor_y = banner_y + banner_y_s;

    const char *os_name = OS_DEFNAME;
    for (int i = 0; os_name[i]; i++) {
        putchar(os_name[i], BANNER_TEXT_COLOR);
    }

    // centered = "console"
    // TODO:
    // should be called after the current app
    const char *center_text = CONSOLE_APP_NAME;
    int text_width = str_len(center_text) * (fm_get_char_width() * font_scale);
    cursor_x = (fb_w - text_width) / 2;
    cursor_y = banner_y + banner_y_s;

    for (int i = 0; center_text[i]; i++) {
        putchar(center_text[i], BANNER_TEXT_COLOR);
    }

    // right side = Date & Time
    banner_update_time();

    // restore cursor and scale from before
    font_scale = o_scale;
    cursor_x = o_cursor_x;
    cursor_y = o_cursor_y;

    needs_update = 0;
}

void banner_update_time(void)
{
    u32 fb_w = get_fb_width();
    u32 o_cursor_x = cursor_x;
    u32 o_cursor_y = cursor_y;
    u32 o_scale = font_scale;

    // based on screen scale

    cmos_time_t time;
    cmos_read_time(&time);

    char time_buf[32];
    str_copy(time_buf, "");

    // date as DD/MM/YY
    if (time.month < 10) str_append(time_buf, "0");
    str_append_uint(time_buf, time.month);
    str_append(time_buf, "/");
    if (time.day < 10) str_append(time_buf, "0");
    str_append_uint(time_buf, time.day);
    str_append(time_buf, "/");
    if (time.year < 10) str_append(time_buf, "0");
    str_append_uint(time_buf, time.year);
    //TODO:
    // in future it will be a programm from functions/cmos.c

    str_append(time_buf, " ");

    // time in HH:MM:SS
    if (time.hour < 10) str_append(time_buf, "0");
    str_append_uint(time_buf, time.hour);
    str_append(time_buf, ":");
    if (time.minute < 10) str_append(time_buf, "0");
    str_append_uint(time_buf, time.minute);
    str_append(time_buf, ":");
    if (time.second < 10) str_append(time_buf, "0");
    str_append_uint(time_buf, time.second);
    //same for here its from cmos.c

    int text_len = str_len(time_buf);
    int text_pixel_width = text_len * (fm_get_char_width() * font_scale);

    u32 char_height = fm_get_char_height() * font_scale;
    draw_rect(fb_w - text_pixel_width - 8, banner_y, text_pixel_width + 8, current_banner_height, BANNER_BG_COLOR);

    cursor_x = fb_w - text_pixel_width - 4;
    cursor_y = banner_y + banner_y_s;

    for (int i = 0; time_buf[i]; i++) {
        putchar(time_buf[i], BANNER_TEXT_COLOR);
    }

    //otherwise the time banner will overwrite the line
    draw_rect(fb_w - text_pixel_width - 8, banner_y + current_banner_height - font_scale,
              text_pixel_width + 8, font_scale, BANNER_BORDER_COLOR);

    font_scale = o_scale;
    cursor_x = o_cursor_x;
    cursor_y = o_cursor_y;
}

// private timer callback (c static)
static void banner_timer_callback(void)
{
    cmos_time_t time;
    cmos_read_time(&time);

    if (time.second != last_second) {
        last_second = time.second;
        needs_update = 1;
    }
    if (needs_update) {
        banner_update_time();
        needs_update = 0;
    }
}

void banner_tick(void) {
    // This function still exists due compatibility
    // i don't want to rename it in other functions too
    banner_timer_callback();
}

void banner_force_update(void) {
    needs_update = 1;
    banner_draw();
}

u32 banner_get_height(void) {
    return current_banner_height;
}

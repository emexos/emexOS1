#include "console.h"
#include <kernel/arch/x64/exceptions/timer.h>
#include <kernel/graph/fm.h>

static int cursor_visible = 1;
static u32 cursor_blink_counter = 0;
static int cursor_enabled = 1;

//todo:
// move to doccr.h with custom cursor bitmap/ character (e.g: A,B, _, █)
#define CURSOR_BLINK_RATE 300 // blinking
//------------------------
// "_"
#define CURSOR_WIDTH_RATIO 6
#define CURSOR_HEIGHT_RATIO 1
//------------------------
// "█"
//#define CURSOR_WIDTH_RATIO 5
//#define CURSOR_HEIGHT_RATIO 8
//------------------------
// "|"
//#define CURSOR_WIDTH_RATIO 1
///#define CURSOR_HEIGHT_RATIO 8
//------------------------
static void cursor_timer_callback() {
    if (!cursor_enabled) return;

    cursor_blink_counter++;
    if (cursor_blink_counter >= CURSOR_BLINK_RATE) {
        cursor_blink_counter = 0;
        cursor_visible = !cursor_visible;
        cursor_redraw();
    }
}

void cursor_(void) {
    cursor_visible = 1;
    cursor_blink_counter = 0;
    cursor_enabled = 1;
    timer_register_callback(cursor_timer_callback);
}

void cursor_draw(void) {
    if (!cursor_visible || !cursor_enabled) return;

    u32 char_width = fm_get_char_width() * font_scale;
    u32 char_height = fm_get_char_height() * font_scale;

    u32 cursor_width = (char_width * CURSOR_WIDTH_RATIO) / 8;
    u32 cursor_height = (char_height * CURSOR_HEIGHT_RATIO) / 8;
    u32 cursor_y_pos = cursor_y + char_height - cursor_height;

    draw_rect(cursor_x, cursor_y_pos, cursor_width, cursor_height, CONSOLESCREEN_COLOR);
}

void cursor_c(void) {
    u32 char_width = fm_get_char_width() * font_scale;
    u32 char_height = fm_get_char_height() * font_scale;

    u32 cursor_width = (char_width * CURSOR_WIDTH_RATIO) / 8;
    u32 cursor_height = (char_height * CURSOR_HEIGHT_RATIO) / 8;
    u32 cursor_y_pos = cursor_y + char_height - cursor_height;

    draw_rect(cursor_x, cursor_y_pos, cursor_width, cursor_height, CONSOLESCREEN_BG_COLOR);
}

void cursor_redraw(void) {
    cursor_c();
    cursor_draw();
}

void cursor_enable(void) {
    cursor_enabled = 1;
    cursor_visible = 1;
    cursor_blink_counter = 0;
}

void cursor_disable(void) {
    cursor_c();
    cursor_enabled = 0;
}

void cursor_reset_blink(void) {
    cursor_blink_counter = 0;
    cursor_visible = 1;
}

#include "console.h"
#include <kernel/arch/x64/exceptions/timer.h>

static int cursor_visible = 1;
static u32 cursor_blink_counter = 0;
static int cursor_enabled = 1;

//todo:
// move to doccr.h with custom cursor bitmap/ character (e.g: A,B, _, █)
#define CURSOR_BLINK_RATE 300 // blinking
//------------------------
// "_"
#define CURSOR_WIDTH 6
#define CURSOR_HEIGHT 1
//------------------------
// "█"
//#define CURSOR_WIDTH 5
//#define CURSOR_HEIGHT 8
//------------------------
// "|"
//#define CURSOR_WIDTH 1
///#define CURSOR_HEIGHT 8
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

    u32 char_width = CURSOR_WIDTH * font_scale;
    u32 char_height = 8 * font_scale;
    u32 cursor_y_pos = cursor_y + char_height - (CURSOR_HEIGHT * font_scale);

    // this is the cursor
    draw_rect(cursor_x, cursor_y_pos, char_width, CURSOR_HEIGHT * font_scale, CONSOLESCREEN_COLOR);
}

void cursor_c(void) {
    u32 char_width = CURSOR_WIDTH * font_scale;
    u32 char_height = 8 * font_scale;
    u32 cursor_y_pos = cursor_y + char_height - (CURSOR_HEIGHT * font_scale);

    // same cursor but inverted color
    draw_rect(cursor_x, cursor_y_pos, char_width, CURSOR_HEIGHT * font_scale, CONSOLESCREEN_BG_COLOR);
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

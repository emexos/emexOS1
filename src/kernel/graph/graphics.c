#include "graphics.h"
#include <kernel/module/module.h>
#include <kernel/communication/serial.h>

//donnot put static before the uints!
u32 *framebuffer = NULL;
u32 fb_width = 0;
u32 fb_height = 0;
u32 fb_pitch = 0;
u32 cursor_x = 16; // 8 = space-character means 8+ text
u32 cursor_y = 16;
u32 font_scale = 0; //for console scaling


void graphics_init(struct limine_framebuffer *fb)
{
    printf("\n");
    framebuffer = (u32 *)fb->address;;
    fb_width = fb->width;
    fb_height = fb->height;
    fb_pitch = fb->pitch;
    cursor_y = 0;
    cursor_x = 0;
    font_scale = 0;

    printbs("Welcome to doccrOS \n", GFX_WHITE);
    printbs("v0.0.1 (alpha)\n", GFX_WHITE);

    printbs("Graphics\n", GFX_WHITE);
    char res_buf[128]; //donot set this to 64
    str_copy(res_buf, "\nFramebuffer ->\n");
    str_append(res_buf, " \t Width: ");
    str_append_uint(res_buf, fb_width);
    str_append(res_buf, "\n");
    str_append(res_buf, " \t Height: ");
    str_append_uint(res_buf, fb_height);
    str_append(res_buf, "\n");
    str_append(res_buf, " \t Pitch: ");
    str_append_uint(res_buf, fb_pitch);
    str_append(res_buf, "\n");
    str_append(res_buf, " \t adress: ");
    str_append_uint(res_buf, (u64) framebuffer);
    str_append(res_buf, "\n");
    printbs(res_buf, GFX_WHITE);
}

void clear(u32 color)
{
    u32 w = get_fb_width();
    u32 h = get_fb_height();
    draw_rect(0, 0, w, h, color);
    reset_cursor();
    print(" ", GFX_BG);
}

void scroll_up(u32 lines)
{
    u32 pixels_to_scroll = lines;
    u32 pitch_dwords = fb_pitch / 4;

    // Move framebuffer content up
    for (u32 y = pixels_to_scroll; y < fb_height; y++) {
        for (u32 x = 0; x < fb_width; x++) {
            framebuffer[(y - pixels_to_scroll) * pitch_dwords + x] =
                framebuffer[y * pitch_dwords + x];
        }
    }

    // Clear bottom lines
    for (u32 y = fb_height - pixels_to_scroll; y < fb_height; y++) {
        for (u32 x = 0; x < fb_width; x++) {
            framebuffer[y * pitch_dwords + x] = CONSOLESCREEN_BG_COLOR;
        }
    }
}

void putpixel(u32 x, u32 y, u32 color)
{
    if (x < fb_width && y < fb_height) {
        framebuffer[y * (fb_pitch / 4) + x] = color;
    }
}

u32 get_fb_width(void){
    return fb_width;
}

u32 get_fb_height(void){
    return fb_height;
}

u32* get_framebuffer(void){
    return framebuffer;
}

u32 get_fb_pitch(void){
    return fb_pitch;
}

void graphics_set_font_scale(u32 scale) {
    if (scale >= 1 && scale <= 4) {
        font_scale = scale;
    }
}

u32 graphics_get_font_scale(void) {
    return font_scale;
}

void set_font_scale(u32 scale) {
    if (scale >= 1 && scale <= 4) {
        font_scale = scale;
    }
}

u32 get_font_scale(void) {
    return font_scale;
}

/*void reset_cursor(void)
{
    cursor_x = 10;
    cursor_y = 10;
    }*/

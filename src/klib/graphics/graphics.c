#include "graphics.h"
#include "../../../shared/types.h"
#include "../../../shared/theme/stdclrs.h"
//#include "standard/screen.h"
#include "../string/string.h"
#include "../string/print.h"

//donnot put static before the uints!
u32 *framebuffer = NULL;
u32 fb_width = 0;
u32 fb_height = 0;
u32 fb_pitch = 0;
u32 cursor_x = 20;
u32 cursor_y = 20;

void graphics_init(struct limine_framebuffer *fb)
{
    framebuffer = (u32 *)fb->address;
    fb_width = fb->width;
    fb_height = fb->height;
    fb_pitch = fb->pitch;

    //cursor_x = 20;
    cursor_y = 20;

    // background rect (test)
    //draw_rect(10, 10, fb_width - 20, fb_height - 20, GFX_BG);

    print("Welcome to doccrOS \n", GFX_WHITE);
    print("v0.0.1 (alph)\n", GFX_WHITE);

    print("Graphics\n", GFX_WHITE);
    char res_buf[64];
    str_copy(res_buf, "Framebuffer ");
    str_append_uint(res_buf, fb_width);
    str_append(res_buf, "x");
    str_append_uint(res_buf, fb_height);
    print(res_buf, GFX_WHITE);
}

void clear(u32 color)
{
    u32 w = get_fb_width();
    u32 h = get_fb_height();
    draw_rect(0, 0, w, h, color);
    reset_cursor();
    print(" ", GFX_BG);
}

void putpixel(u32 x, u32 y, u32 color)
{
    if (x < fb_width && y < fb_height) {
        framebuffer[y * (fb_pitch / 4) + x] = color;
    }
}
//move to draw.c later

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

/*void reset_cursor(void)
{
    cursor_x = 10;
    cursor_y = 10;
    }*/

#ifndef GRAPHICS_H
#define GRAPHICS_H

extern u32 *framebuffer;
extern u32 fb_width;
extern u32 fb_height;
extern u32 fb_pitch;
extern u32 cursor_x;
extern u32 cursor_y;

#include "../../../shared/types.h"
#include "../../../third_party/limine/limine.h"

void graphics_init(struct limine_framebuffer *fb);
void putpixel(u32 x, u32 y, u32 color);

u32 get_fb_width(void);
u32 get_fb_height(void);
u32* get_framebuffer(void);
u32 get_fb_pitch(void);

void reset_cursor(void);
void clear(u32 color);

void draw_rect(u32 x, u32 y, u32 width, u32 height, u32 color);
void draw_circle(u32 cx, u32 cy, u32 radius, u32 color);
void draw_line(u32 x0, u32 y0, u32 x1, u32 y1, u32 color);

#endif

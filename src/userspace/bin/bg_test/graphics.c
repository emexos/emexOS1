#include "gen.h"

void blit_row(int x, int y, int w, unsigned int *px) {
    fb_rect_t r = {
        .x = (unsigned)x,
        .y = (unsigned)y,
        .w = (unsigned)w,
        .h = 1,
        .pixels = px
    };
    ioctl(fb, FBIO_BLIT, &r);
}

unsigned int checker(int x, int y) {
    return (((x / CHECKER_SIZE) ^ (y / CHECKER_SIZE)) & 1) ? GRAY_D : GRAY_L;
}

// fill a region with the background pattern
void blit_rect_bg(int x, int y, int w, int h) {
    for (int dy = 0; dy < h; dy++) {
        int sy = y + dy;
        if (sy < 0 || sy >= scr_h) continue;
        for (int dx = 0; dx < w && dx < ROW_BUF_W; dx++)
        {
            int sx = x + dx;

            #if WINDOWS95 == 1
            	row_buf[dx] = WIN95_TEAL;
            #elif BG_MACOS == 1
            	row_buf[dx] = (sx >= 0 && sx < scr_w) ? checker(sx, sy) : GRAY_L;
            #else
            	row_buf[dx] = GRAY_D;
            #endif
        }
        blit_row(x, sy, w, row_buf);
    }
}

void graphics_init(void) {
    fb_var_screeninfo vinfo;
    ioctl(fb, FBIOGET_VSCREENINFO, &vinfo);
    scr_w = (int)(vinfo.xres > 0 ? vinfo.xres : 1024);
    scr_h = (int)(vinfo.yres > 0 ? vinfo.yres : 768);

    fb_fix_screeninfo finfo;
    ioctl(fb, FBIOGET_FSCREENINFO, &finfo);
    scr_pitch = (int)(finfo.line_length > 0 ? finfo.line_length : scr_w * 4);
}
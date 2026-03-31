#include "cursor.h"
#include "../compositor/comp.h"

#include <sys/ioctl.h>
#include <emx/fb.h>

#define CW CUR_W
#define CH CUR_H

// color config
#define T 0x00000000u
#define B 0xFF101010u
#define W 0xFFFFFFFFu

static const unsigned int cur_px[CW * CH] =
{
    B,T,T,T,T,T,T,T,T,T,T,T,
    B,B,T,T,T,T,T,T,T,T,T,T,
    B,W,B,T,T,T,T,T,T,T,T,T,
    B,W,W,B,T,T,T,T,T,T,T,T,
    B,W,W,W,B,T,T,T,T,T,T,T,
    B,W,W,W,W,B,T,T,T,T,T,T,
    B,W,W,W,W,W,B,T,T,T,T,T,
    B,W,W,W,W,W,W,B,T,T,T,T,
    B,W,W,W,W,W,W,W,B,T,T,T,
    B,W,W,W,W,W,W,W,W,B,T,T,
    B,W,W,W,W,W,W,W,W,W,B,T,
    B,W,W,W,W,B,B,B,B,B,B,T,
    B,W,W,W,B,T,T,T,T,T,T,T,
    B,W,W,B,T,T,T,T,T,T,T,T,
    B,W,B,T,T,T,T,T,T,T,T,T,
    B,B,T,T,T,T,T,T,T,T,T,T,
};

static unsigned int bg_save[CW * CH];
static int bg_valid = 0;
static int old_x = 0;
static int old_y = 0;
static int g_fd  = -1;
static int g_scr_w = 0;
static int g_scr_h = 0;

static void clamp(int *x, int *y)
{
    if (*x < 0) *x = 0;
    if (*y < 0) *y = 0;
    if (*x + CW > g_scr_w) *x = g_scr_w - CW;
    if (*y + CH > g_scr_h) *y = g_scr_h - CH;
}

void cur_init(int fb_fd, int w, int h)
{
    g_fd = fb_fd;
    g_scr_w = w;
    g_scr_h = h;
    bg_valid = 0;
}

void cur_undo_from_backbuf(void)
{
    if (!bg_valid) return;
    comp_put_pixels(old_x, old_y, CW, CH, bg_save);
}

void cur_bake(int x, int y)
{
    clamp(&x, &y);

    for (int row = 0; row < CH; row++) {
        for (int col = 0; col < CW; col++) {
            bg_save[row * CW + col] = comp_get(x + col, y + row);
        }
    }
    for (int row = 0; row < CH; row++) {
        for (int col = 0; col < CW; col++) {
            unsigned int c = cur_px[row * CW + col];
            if (c >> 24) comp_set(x + col, y + row, c);
        }
    }

    old_x = x;
    old_y = y;
    bg_valid = 1;
}

void cur_erase_fb(void)
{
    if (!bg_valid || g_fd < 0) return;
    fb_rect_t r = {
        .x      = (unsigned)old_x,
        .y      = (unsigned)old_y,
        .w      = CW,
        .h      = CH,
        .pixels = bg_save
    };
    ioctl(g_fd, FBIO_BLIT, &r);
    bg_valid = 0;
}

void cur_draw_fb(int x, int y)
{
    if (g_fd < 0) return;
    clamp(&x, &y);

    // save FB pixels under the cursor
    fb_rect_t save = {
        .x      = (unsigned)x,
        .y      = (unsigned)y,
        .w      = CW,
        .h      = CH,
        .pixels = bg_save
    };
    ioctl(g_fd, FBIO_READ_RECT, &save);

    // draws the cursor to the FB
    fb_rect_t draw = {
        .x      = (unsigned)x,
        .y      = (unsigned)y,
        .w      = CW,
        .h      = CH,
        .pixels = (unsigned int *)cur_px
    };

    ioctl(g_fd, FBIO_BLIT, &draw);

    old_x = x;
    old_y = y;
    bg_valid = 1;
}

int cur_valid(void) { return bg_valid; }
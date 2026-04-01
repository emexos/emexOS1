#include "comp.h"

#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <emx/fb.h>

static int g_fd = -1;
static int g_w = 0;
static int g_h = 0;
static unsigned int *g_buf = 0;

void comp_init(int fb_fd, int w, int h)
{
    g_fd = fb_fd;
    g_w = w;
    g_h = h;
    g_buf = (unsigned int *)malloc((unsigned)(w * h) * sizeof(unsigned int));
}

void comp_capture(void)
{
    if (!g_buf || g_fd < 0) return;
    fb_rect_t r = {
        .x = 0, .y = 0,
        .w = (unsigned)g_w,
        .h = (unsigned)g_h,
        .pixels = g_buf
    };
    ioctl(g_fd, FBIO_READ_RECT, &r);
}

void comp_fill(
	int x, int y,
	int w, int h,
	unsigned int color
) {
    if (!g_buf) return;
    for (int dy = 0; dy < h; dy++)
    {
        int py = y + dy;
        unsigned int *row = g_buf + py * g_w;
        int x0 = x < 0 ? 0 : x;
        int x1 = (x + w) > g_w ? g_w : (x + w);

        if (py < 0 || py >= g_h) continue;

        for (int px = x0; px < x1; px++) row[px] = color;
    }
}

unsigned int comp_get(int x, int y)
{
    if (!g_buf || x < 0 || x >= g_w || y < 0 || y >= g_h) return 0;

    return g_buf[y * g_w + x];
}

void comp_set(int x, int y, unsigned int c)
{
    if (!g_buf || x < 0 || x >= g_w || y < 0 || y >= g_h) return;

    g_buf[y * g_w + x] = c;
}

void comp_put_row(int x, int y, const unsigned int *row, int len)
{
	unsigned int *dst = g_buf + y * g_w;

    if (!g_buf || y < 0 || y >= g_h) return;

    for (int i = 0; i < len; i++)
    {
        int px = x + i;
        if (px >= 0 && px < g_w) dst[px] = row[i];
    }
}

void comp_copy_rect(
	int src_x, int src_y,
    int dst_x, int dst_y,
    int w, int h
) {
    if (!g_buf) return;
    if (w <= 0 || h <= 0) return;

    int step = (dst_y <= src_y) ? 1 : -1;
    int row_start = (step == 1) ? 0 : h - 1;
    int row_end = (step == 1) ? h : -1;
    int xstep = (dst_x <= src_x) ? 1 : -1;
    int col_start = (xstep == 1) ? 0 : w - 1;
    int col_end = (xstep == 1) ? w : -1;

    for (int r = row_start; r != row_end; r += step)
    {
        int sy = src_y + r;
        int dy = dst_y + r;
        if (sy < 0 || sy >= g_h || dy < 0 || dy >= g_h) continue;

        unsigned int *src_row = g_buf + sy * g_w;
        unsigned int *dst_row = g_buf + dy * g_w;

        // clamp x range
        int col_start = (dst_x <= src_x) ? 0 : w - 1;
        int col_end = (dst_x <= src_x) ? w : -1;
        int xstep = (dst_x <= src_x) ? 1 : -1;
        for (int c = col_start; c != col_end; c += xstep)
        {
            int spx = src_x + c;
            int dpx = dst_x + c;

            if (spx >= 0 && spx < g_w && dpx >= 0 && dpx < g_w) dst_row[dpx] = src_row[spx];
        }
    }
}

void comp_put_pixels(
	int x, int y, int w, int h,
    const unsigned int *pixels
) {
    if (!g_buf || !pixels) return;
    for (int row = 0; row < h; row++)
    {
        int py = y + row;

        if (py < 0 || py >= g_h) continue;
        unsigned int *dst = g_buf + py * g_w;

        for (int col = 0; col < w; col++)
        {
            int px = x + col;

            if (px >= 0 && px < g_w) dst[px] = pixels[row * w + col];
        }
    }
}

void comp_flush(void)
{
    if (!g_buf || g_fd < 0) return;
    fb_rect_t r = {
        .x = 0, .y = 0,
        .w = (unsigned)g_w,
        .h = (unsigned)g_h,
        .pixels = g_buf
    };
    ioctl(g_fd, FBIO_BLIT, &r);
}

int comp_w(void)
{
	return g_w;
}

int comp_h(void)
{
	return g_h;
}
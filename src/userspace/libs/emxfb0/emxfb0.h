#pragma once

#include <sys/ioctl.h>
#include <emx/fb.h>

#define DS_ROW_BUF_W 2048

typedef struct {
    int fd;
    int w, h, pitch;
    unsigned int row_buf[DS_ROW_BUF_W];
} draw_ctx;


void ds_init(draw_ctx *ctx, int fb_fd);
void ds_blit_row(draw_ctx *ctx, int x, int y, int len);

void ds_pixel(draw_ctx *ctx, int x, int y, int w, int h, unsigned int color);
void ds_hline(draw_ctx *ctx, int x, int y, int w, unsigned int color);
void ds_vline(draw_ctx *ctx, int x, int y, int h, unsigned int color);
void ds_rect(draw_ctx *ctx, int x, int y, int w, int h, unsigned int color);
void ds_rect_outline(draw_ctx *ctx, int x, int y, int w, int h, unsigned int color);
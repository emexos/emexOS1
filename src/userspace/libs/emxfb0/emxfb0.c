#include "emxfb0.h"

void ds_blit_row(draw_ctx *ctx, int x, int y, int len) {
    fb_rect_t r = {
        .x = (unsigned)x,
        .y = (unsigned)y,
        .w = (unsigned)len,
        .h = 1,
        .pixels = ctx->row_buf
    };
    ioctl(ctx->fd, FBIO_BLIT, &r);
}

void ds_pixel(draw_ctx *ctx, int x, int y, int w, int h, unsigned int color) {
    (void)w; (void)h;
    if (x < 0 || y < 0 || x >= ctx->w || y >= ctx->h) return;
    ctx->row_buf[0] = color;
    fb_rect_t r = { (unsigned)x, (unsigned)y, 1, 1, ctx->row_buf };
    ioctl(ctx->fd, FBIO_BLIT, &r);
}

void ds_hline(draw_ctx *ctx, int x, int y, int w, unsigned int color) {
    if (y < 0 || y >= ctx->h) return;
    if (x < 0) { w += x; x = 0; }
    if (x + w > ctx->w) w = ctx->w - x;
    if (w <= 0) return;

    for (int i = 0; i < w && i < DS_ROW_BUF_W; i++)
        ctx->row_buf[i] = color;
    ds_blit_row(ctx, x, y, w);
}

void ds_vline(draw_ctx *ctx, int x, int y, int h, unsigned int color) {
    for (int dy = 0; dy < h; dy++) {
        int sy = y + dy;
        if (sy < 0 || sy >= ctx->h || x < 0 || x >= ctx->w) continue;
        ctx->row_buf[0] = color;
        fb_rect_t r = { (unsigned)x, (unsigned)sy, 1, 1, ctx->row_buf };
        ioctl(ctx->fd, FBIO_BLIT, &r);
    }
}

void ds_rect(draw_ctx *ctx, int x, int y, int w, int h, unsigned int color){
    for (int dy = 0; dy < h; dy++)
        ds_hline(ctx, x, y + dy, w, color);
}

void ds_rect_outline(draw_ctx *ctx, int x, int y, int w, int h, unsigned int color) {
    ds_hline(ctx, x, y, w, color); // top
    ds_hline(ctx, x, y + h - 1, w, color); // bottom
    ds_vline(ctx, x, y, h, color); // left
    ds_vline(ctx, x + w - 1, y, h, color); // right
}
void ds_init(draw_ctx *ctx, int fb_fd)
{
    ctx->fd = fb_fd;

    fb_var_screeninfo vinfo;
    ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);
    ctx->w = (int)(vinfo.xres > 0 ? vinfo.xres : 1024);
    ctx->h = (int)(vinfo.yres > 0 ? vinfo.yres : 768);

    fb_fix_screeninfo finfo;
    ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo);
    ctx->pitch = (int)(finfo.line_length > 0 ? finfo.line_length : ctx->w * 4);
}
#include "vad.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

// def. 256color ARGB
// entries: 0xAARRGGBB
// 00 is transparent (0x)

static unsigned int _default_palette[VAD_PALETTE_SIZE] = {
    /* 0x00 */ 0x00000000u,  /* transparent  */

    /* 0x01 */ 0xFF0A0A0Au,
    /* 0x02 */ 0xFF141414u,
    /* 0x03 */ 0xFF1E1E1Eu,
    /* 0x04 */ 0xFF282828u,
    /* 0x05 */ 0xFF323232u,
    /* 0x06 */ 0xFF3C3C3Cu,
    /* 0x07 */ 0xFF464646u,
    /* 0x08 */ 0xFF505050u,
    /* 0x09 */ 0xFF5A5A5Au,
    /* 0x0A */ 0xFF646464u,
    /* 0x0B */ 0xFF6E6E6Eu,
    /* 0x0C */ 0xFF787878u,
    /* 0x0D */ 0xFF828282u,
    /* 0x0E */ 0xFF8C8C8Cu,
    /* 0x0F */ 0xFF181818u,  /* dark;detail  */

    /* 0x10 */ 0xFF969696u,
    /* 0x11 */ 0xFFA0A0A0u,
    /* 0x12 */ 0xFFD8D8D8u,  /* light edge*/
    /* 0x13 */ 0xFFECECECu,  /* lightest*/
    /* 0x14 */ 0xFFA8A8A8u,  /* shadow edge */
    /* 0x15 */ 0xFF787878u,  /* deep shadow */
    /* 0x16 */ 0xFFB4B4B4u,
    /* 0x17 */ 0xFFBEBEBEu,
    /* 0x18 */ 0xFFC8C8C8u,
    /* 0x19 */ 0xFFD2D2D2u,
    /* 0x1A */ 0xFFDCDCDCu,
    /* 0x1B */ 0xFFE6E6E6u,
    /* 0x1C */ 0xFFF0F0F0u,
    /* 0x1D */ 0xFFFAFAFAu,

    /* 0x1E */ 0xFFFFFFFFu,
    /* 0x1F */ 0xFF000000u,

    // 6×6×6 colour cube (216 colours) + extended grays
    // https://en.wikipedia.org/wiki/List_of_color_palettes
    // R=ri*51, G=gi*51, B=bi*51
    //
    // where ri,gi,bi elemt {0,1,2,3,4,5}
    // index = 0x20 + ri*36+gi*6+bi

#define _C(r,g,b) (0xFF000000u|(((unsigned)(r))<<16)|(((unsigned)(g))<<8)|((unsigned)(b)))
    _C(  0,  0,  0), _C(  0,  0, 51), _C(  0,  0,102), _C(  0,  0,153),
    _C(  0,  0,204), _C(  0,  0,255), _C(  0, 51,  0), _C(  0, 51, 51),
    _C(  0, 51,102), _C(  0, 51,153), _C(  0, 51,204), _C(  0, 51,255),
    _C(  0,102,  0), _C(  0,102, 51), _C(  0,102,102), _C(  0,102,153),
    _C(  0,102,204), _C(  0,102,255), _C(  0,153,  0), _C(  0,153, 51),
    _C(  0,153,102), _C(  0,153,153), _C(  0,153,204), _C(  0,153,255),
    _C(  0,204,  0), _C(  0,204, 51), _C(  0,204,102), _C(  0,204,153),
    _C(  0,204,204), _C(  0,204,255), _C(  0,255,  0), _C(  0,255, 51),
    _C(  0,255,102), _C(  0,255,153), _C(  0,255,204), _C(  0,255,255),

    _C( 51,  0,  0), _C( 51,  0, 51), _C( 51,  0,102), _C( 51,  0,153),
    _C( 51,  0,204), _C( 51,  0,255), _C( 51, 51,  0), _C( 51, 51, 51),
    _C( 51, 51,102), _C( 51, 51,153), _C( 51, 51,204), _C( 51, 51,255),
    _C( 51,102,  0), _C( 51,102, 51), _C( 51,102,102), _C( 51,102,153),
    _C( 51,102,204), _C( 51,102,255), _C( 51,153,  0), _C( 51,153, 51),
    _C( 51,153,102), _C( 51,153,153), _C( 51,153,204), _C( 51,153,255),
    _C( 51,204,  0), _C( 51,204, 51), _C( 51,204,102), _C( 51,204,153),
    _C( 51,204,204), _C( 51,204,255), _C( 51,255,  0), _C( 51,255, 51),
    _C( 51,255,102), _C( 51,255,153), _C( 51,255,204), _C( 51,255,255),

    _C(102,  0,  0), _C(102,  0, 51), _C(102,  0,102), _C(102,  0,153),
    _C(102,  0,204), _C(102,  0,255), _C(102, 51,  0), _C(102, 51, 51),
    _C(102, 51,102), _C(102, 51,153), _C(102, 51,204), _C(102, 51,255),
    _C(102,102,  0), _C(102,102, 51), _C(102,102,102), _C(102,102,153),
    _C(102,102,204), _C(102,102,255), _C(102,153,  0), _C(102,153, 51),
    _C(102,153,102), _C(102,153,153), _C(102,153,204), _C(102,153,255),
    _C(102,204,  0), _C(102,204, 51), _C(102,204,102), _C(102,204,153),
    _C(102,204,204), _C(102,204,255), _C(102,255,  0), _C(102,255, 51),
    _C(102,255,102), _C(102,255,153), _C(102,255,204), _C(102,255,255),

    _C(153,  0,  0), _C(153,  0, 51), _C(153,  0,102), _C(153,  0,153),
    _C(153,  0,204), _C(153,  0,255), _C(153, 51,  0), _C(153, 51, 51),
    _C(153, 51,102), _C(153, 51,153), _C(153, 51,204), _C(153, 51,255),
    _C(153,102,  0), _C(153,102, 51), _C(153,102,102), _C(153,102,153),
    _C(153,102,204), _C(153,102,255), _C(153,153,  0), _C(153,153, 51),
    _C(153,153,102), _C(153,153,153), _C(153,153,204), _C(153,153,255),
    _C(153,204,  0), _C(153,204, 51), _C(153,204,102), _C(153,204,153),
    _C(153,204,204), _C(153,204,255), _C(153,255,  0), _C(153,255, 51),
    _C(153,255,102), _C(153,255,153), _C(153,255,204), _C(153,255,255),

    _C(204,  0,  0), _C(204,  0, 51), _C(204,  0,102), _C(204,  0,153),
    _C(204,  0,204), _C(204,  0,255), _C(204, 51,  0), _C(204, 51, 51),
    _C(204, 51,102), _C(204, 51,153), _C(204, 51,204), _C(204, 51,255),
    _C(204,102,  0), _C(204,102, 51), _C(204,102,102), _C(204,102,153),
    _C(204,102,204), _C(204,102,255), _C(204,153,  0), _C(204,153, 51),
    _C(204,153,102), _C(204,153,153), _C(204,153,204), _C(204,153,255),
    _C(204,204,  0), _C(204,204, 51), _C(204,204,102), _C(204,204,153),
    _C(204,204,204), _C(204,204,255), _C(204,255,  0), _C(204,255, 51),
    _C(204,255,102), _C(204,255,153), _C(204,255,204), _C(204,255,255),

    _C(255,  0,  0), _C(255,  0, 51), _C(255,  0,102), _C(255,  0,153),
    _C(255,  0,204), _C(255,  0,255), _C(255, 51,  0), _C(255, 51, 51),
    _C(255, 51,102), _C(255, 51,153), _C(255, 51,204), _C(255, 51,255),
    _C(255,102,  0), _C(255,102, 51), _C(255,102,102), _C(255,102,153),
    _C(255,102,204), _C(255,102,255), _C(255,153,  0), _C(255,153, 51),
    _C(255,153,102), _C(255,153,153), _C(255,153,204), _C(255,153,255),
    _C(255,204,  0), _C(255,204, 51), _C(255,204,102), _C(255,204,153),
    _C(255,204,204), _C(255,204,255), _C(255,255,  0), _C(255,255, 51),
    _C(255,255,102), _C(255,255,153), _C(255,255,204), _C(255,255,255),
#undef _C
};

static const unsigned int *_active_palette = _default_palette;

void vad_set_palette(const unsigned int *palette) {
    _active_palette = palette ? palette : _default_palette;
}

const unsigned int *vad_get_palette(void) {
    return _active_palette;
}

void vad_set_color(int index, unsigned int argb)
{
    if (index < 0 || index >= VAD_PALETTE_SIZE) return;
    // only works when the active palette is the default (when writable)
    //
    if (
    	_active_palette == _default_palette
    )
    	_default_palette[index] = argb;
}



static unsigned int _clamp(int v) {
    if (v < 0) return 0;
    if (v > 255) return 255;
    return (unsigned int)v;
}
static int _read_exact(int fd, void *buf, int n)
{
    unsigned char *p = (unsigned char *)buf;
    int remaining = n;
    while (remaining > 0)
    {
        int r = (int)read(fd, p, (size_t)remaining);
        if (r <= 0) return -1;

        p += r;

        remaining -= r;
    }
    return 0;
}
static unsigned int _apply_fx(unsigned int argb, int sat, int bright, int alpha)
{
    unsigned int a = (argb >> 24) & 0xFF;
    int r = (int)((argb >> 16) & 0xFF);
    int g = (int)((argb >>  8) & 0xFF);
    int b = (int)( argb & 0xFF);

    if (sat != -1)
    {
        int gray = (r * 77 + g * 151 + b * 28) >> 8;
        r = gray + (((r - gray) * sat) >> 8);
        g = gray + (((g - gray) * sat) >> 8);
        b = gray + (((b - gray) * sat) >> 8);
        r = (int)_clamp(r);
        g = (int)_clamp(g);
        b = (int)_clamp(b);
    }
    if (bright != 0)
    {
        r = (int)_clamp(r + bright);
        g = (int)_clamp(g + bright);
        b = (int)_clamp(b + bright);
    }
    if (alpha != 255) a = (a * (unsigned int)alpha) / 255u;

    return (a << 24) | ((unsigned int)r << 16) | ((unsigned int)g << 8) | (unsigned int)b;
}

// palette decode to ARGB pixels (top down)
static void _decode(vad_image_t *img)
{
 	int w = img->width;
    int h = img->height;

    if (!img || !img->indices || !img->pixels) return;

    for (int y = 0; y < h; y++)
    {
    //top-down convert
        int src_y = h - 1 - y;
        for (int x = 0; x < w; x++) {
            unsigned char idx = img->indices[src_y * w + x];
            img->pixels[y * w + x] = _active_palette[idx];
        }
    }
}

int vad_load(const char *path, vad_image_t *img)
{
    if (!path || !img) return -1;

    img->indices = NULL;
    img->pixels = NULL;
    img->width = 0;
    img->height = 0;
    unsigned short w_le, h_le;
    int fd = open(path, O_RDONLY);
    int w = (int)w_le;
    int h = (int)h_le;
    char magic[4];

    if (fd < 0) return -1;

    // will always be the start of a file
    if (_read_exact(fd, magic, 4) != 0) goto fail;
    if (magic[0] != 'T' || magic[1] != 'G' ||
        magic[2] != 'R' || magic[3] != 'V') goto fail;
    /* example:
    TGRV((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((
     */

    // dimensions
    if (_read_exact(fd, &w_le, 2) != 0) goto fail;
    if (_read_exact(fd, &h_le, 2) != 0) goto fail;

    if (w <= 0 || h <= 0 || w > 1024 || h > 1024) goto fail;

    //alocate
    img->indices =(unsigned char *)malloc((size_t)(w * h));
    img->pixels =(unsigned int *)malloc((size_t)(w * h) * 4);

    if (!img->indices || !img->pixels) goto fail2;

    img->width = w;
    img->height = h;

    // reads raw index
    if (_read_exact(fd, img->indices, w * h) != 0) goto fail2;

    close(fd);

    _decode(img); //(AARRGGBB)
    return 0;

fail2:
    free(img->indices); img->indices = NULL;
    free(img->pixels); img->pixels = NULL;
fail:
    close(fd);
    return -1;
}

void vad_reload_pixels(vad_image_t *img)
{
    _decode(img);
}

void vad_free(vad_image_t *img)
{
    if (!img) return;

    free(img->indices); img->indices = NULL;
    free(img->pixels); img->pixels = NULL;

    img->width = img->height = 0;
}

void vad_draw(draw_ctx *ctx, const vad_image_t *img, int x, int y)
{
    vad_draw_ex(ctx, img, x, y, 0, 0, -1, 0, 255);
}

void vad_draw_scaled(
	draw_ctx *ctx, const vad_image_t *img,
   	int x, int y, int w, int h)
{
    vad_draw_ex(ctx, img, x, y, w, h, -1, 0, 255);
}

void vad_draw_ex(
	draw_ctx *ctx, const vad_image_t *img,
    int x, int y, int w, int h,
    int sat, int bright, int alpha
){
    if (!ctx || !img || !img->pixels) return;
    if (alpha == 0) return;

    int src_w = img->width;
    int src_h = img->height;
    int dst_w = (w > 0) ? w : src_w;
    int dst_h = (h > 0) ? h : src_h;

    int has_fx = (sat != -1 || bright != 0 || alpha != 255);

    for (int dy = 0; dy < dst_h; dy++)
    {
        int abs_y = y + dy;
        if (abs_y < 0 || abs_y >= ctx->h) continue;

        int sy = (dst_h > 1) ? (dy * (src_h - 1) / (dst_h - 1)) : 0;
        if (sy >= src_h) sy = src_h - 1;

        int row_len = dst_w;
        if (row_len > DS_ROW_BUF_W) row_len = DS_ROW_BUF_W;

        for (int dx = 0; dx < row_len; dx++)
        {
            int sx = (dst_w > 1) ? (dx * (src_w - 1) / (dst_w - 1)) : 0;
            if (sx >= src_w) sx = src_w - 1;

            unsigned int pix = img->pixels[sy * src_w + sx];

            if (has_fx) pix = _apply_fx(pix, sat, bright, alpha);

            ctx->row_buf[dx] = pix;
        }

        ds_blit_row(ctx, x, abs_y, row_len);
    }

}
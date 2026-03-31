#include "bmp.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    unsigned char sig[2];      /* 'B', 'M'                      */
    unsigned int file_size;    /*total file size in bytes       */
    unsigned short reserved1;
    unsigned short reserved2;
    unsigned int data_offset;  /* byte offset to pixel array     */
} __attribute__((packed)) _bmp_fhdr_t;

typedef struct
{
    unsigned int hdr_size;
    int width;
    int height;

    unsigned short planes;
    unsigned short bpp;      //24 or 32
    unsigned int compression;// 0 = none ; 3 = bitfields
    unsigned int image_size;

    int xppm;
    int yppm;

    unsigned int clr_used;
    unsigned int clr_important;
} __attribute__((packed)) _bmp_dib_t;


static unsigned int _clamp(int v)
{
    if (v < 0)   return 0;
    if (v > 255) return 255;
    return (unsigned int)v;
}
static int _read_exact(int fd, void *buf, int n)
{
    unsigned char *p = (unsigned char *)buf;
    int remaining = n;
    while (remaining > 0) {
        int r = (int)read(fd, p, (size_t)remaining);
        if (r <= 0) return -1;

        p += r;

        remaining -= r;
    }
    return 0;
}
static int _skip(int fd, int n)
{
    unsigned char tmp[64];
    while (n > 0) {
        int chunk = (n > 64) ? 64 : n;
        int r = (int)read(fd, tmp, (size_t)chunk);
        if (r <= 0) return -1;
        n -= r;
    }
    return 0;
}
static unsigned int _apply_fx(unsigned int argb, int sat, int bright, int alpha)
{
    unsigned int a = (argb >> 24) & 0xFF;
    int r = (int)((argb >> 16) & 0xFF);
    int g = (int)((argb >> 8 ) & 0xFF);

    int b = (int)(argb  & 0xFF);

    //saturation
    if (sat != -1)
    {
    	//https://en.wikipedia.org/wiki/Grayscale
     	// https://en.wikipedia.org/wiki/Colorfulness
        // Y = 0.30R + 0.59G + 0.11B  (scaled x256)
        int gray = (r * 77 + g * 151 + b * 28) >> 8;
        r = gray + (((r - gray) * sat) >> 8);
        g = gray + (((g - gray) * sat) >> 8);
        b = gray + (((b - gray) * sat) >> 8);
        r = (int)_clamp(r);
        g = (int)_clamp(g);
        b = (int)_clamp(b);
    }

    //b rightnes
    if (bright != 0)
    {
        r = (int)_clamp(r + bright);
        g = (int)_clamp(g + bright);
        b = (int)_clamp(b + bright);
    }

    // 0x00 is transparent btw
    if (alpha != 255)
    {
        a = (a * (unsigned int)alpha) / 255u;
    }

    return (a << 24) | ((unsigned int)r << 16) | ((unsigned int)g << 8) | (unsigned int)b;

}


int bmp_load(const char *path, bmp_image_t *img)
{
    if (!path || !img) return -1;

    img->pixels = NULL;
    img->width = 0;
    img->height = 0;

    _bmp_fhdr_t fhdr;
    _bmp_dib_t dib;

    int fd = open(path, O_RDONLY);
    int w = dib.width;
    int h = dib.height;
    int top_down = 0;

    if (fd < 0) return -1;
    if (_read_exact(fd, &fhdr, (int)sizeof(fhdr)) != 0) goto fail;
    if (fhdr.sig[0] != 'B' || fhdr.sig[1] != 'M') goto fail;
    if (_read_exact(fd, &dib, (int)sizeof(dib)) != 0) goto fail;
    if (dib.bpp != 24 && dib.bpp != 32) goto fail; // only uncompressed
    if (dib.compression != 0 && dib.compression != 3) goto fail;
    if (h < 0) { h = -h; top_down = 1; }
    if (w <= 0 || h <= 0) goto fail;

    // alocates pixel buffer
    img->pixels = (unsigned int *)malloc((size_t)(w * h) * 4);
    img->width  = w;
    img->height = h;

    if (!img->pixels) goto fail;


    int already_read = (int)(sizeof(fhdr) + sizeof(dib));
    int to_skip = (int)fhdr.data_offset - already_read;
    if (to_skip > 0 && _skip(fd, to_skip) != 0) goto fail2;

    int row_bytes = (dib.bpp == 24)
        ? ((w * 3 + 3) & ~3)   // 24bpp pad 4b
        : (w * 4);             // 32bpp pad N

    unsigned char *row_buf = (unsigned char *)malloc((size_t)row_bytes);
    if (!row_buf) goto fail2;

    for (int y = 0; y < h; y++)
    {
        if (_read_exact(fd, row_buf, row_bytes) != 0) break;

        /* BMP is bottom-up unless top_down */
        int dst_y = top_down ? y : (h - 1 - y);
        unsigned int *dst = img->pixels + dst_y * w;

        for (int x = 0; x < w; x++)
        {
            unsigned int r, g, b, a;
            if (dib.bpp == 24) {
                b = row_buf[x * 3 + 0];
                g = row_buf[x * 3 + 1];
                r = row_buf[x * 3 + 2];
                a = 0xFF;
            } else { /* 32 bpp */
                b = row_buf[x * 4 + 0];
                g = row_buf[x * 4 + 1];
                r = row_buf[x * 4 + 2];
                a = row_buf[x * 4 + 3];

                if (a == 0) a = 0xFF;// some 32bpp bmps store alpha=0 even for opaque pixels
            }

            dst[x] = (a << 24) | (r << 16) | (g << 8) | b;
        }
    }

    free(row_buf);
    close(fd);
    return 0;

fail2:
    free(img->pixels);
    img->pixels = NULL;
fail:
    close(fd);
    return -1;
}

void bmp_free(bmp_image_t *img)
{
    if (!img) return;
    free(img->pixels);
    img->pixels = NULL;
    img->width  = 0;
    img->height = 0;
}

void bmp_draw(draw_ctx *ctx, const bmp_image_t *img, int x, int y)
{
    bmp_draw_ex(ctx, img, x, y, 0, 0, -1, 0, 255);
}

void bmp_draw_scaled(
	draw_ctx *ctx, const bmp_image_t *img,
    int x, int y, int w, int h)
{
    bmp_draw_ex(ctx, img, x, y, w, h, -1, 0, 255);
}

void bmp_draw_ex(
	draw_ctx *ctx, const bmp_image_t *img,
    int x, int y, int w, int h,
    int sat, int bright, int alpha
) {
    if (!ctx || !img || !img->pixels) return;
    if (alpha == 0) return; // fully transparent 0x00

    int src_w = img->width;
    int src_h = img->height;
    int dst_w = (w > 0) ? w : src_w;
    int dst_h = (h > 0) ? h : src_h;

    // cheks for effects
    int has_fx = (sat != -1 || bright != 0 || alpha != 255);

    for (int dy = 0; dy < dst_h; dy++) {
        int abs_y = y + dy;
        int sy = (dst_h > 1) ? (dy * (src_h - 1) / (dst_h - 1)) : 0;
        int row_len = dst_w;

        if (abs_y < 0 || abs_y >= ctx->h) continue;
        if (sy >= src_h) sy = src_h - 1;
        if (row_len > DS_ROW_BUF_W) row_len = DS_ROW_BUF_W;

        for (int dx = 0; dx < row_len; dx++)
        {
            // nearest neighboor
            int sx = (dst_w > 1) ? (dx * (src_w - 1) / (dst_w - 1)) : 0;
            if (sx >= src_w) sx = src_w - 1;

            unsigned int pix = img->pixels[sy * src_w + sx];

            if (has_fx) pix = _apply_fx(pix, sat, bright, alpha);

            ctx->row_buf[dx] = pix;
        }

        // ds_blit_row goes to FBIO_BLIT which skips pixels where alpha == 0
        ds_blit_row(ctx, x, abs_y, row_len);
    }
}
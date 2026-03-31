#pragma once

/*
 * 24bpp and 32bpp , uncompressed BMP
 *
 */

#include "../emxfb0/emxfb0.h"

typedef struct {
    int width;
    int height;
    unsigned int *pixels; /* ARGB 0xAARRGGBB; top-down */
} bmp_image_t;


int bmp_load(const char *path, bmp_image_t *img);

void bmp_free(bmp_image_t *img);

void bmp_draw(draw_ctx *ctx, const bmp_image_t *img, int x, int y);

void bmp_draw_scaled(
	draw_ctx *ctx, const bmp_image_t *img,
    int x, int y, int w, int h
);

void bmp_draw_ex(
	draw_ctx *ctx, const bmp_image_t *img,
    int x, int y, int w, int h,
    int sat, int bright, int alpha
);
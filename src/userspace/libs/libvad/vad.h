#pragma once

#include "../emxfb0/emxfb0.h"

#define VAD_PALETTE_SIZE 256
#define VAD_MAGIC "TGRV" //54 47 52 56

typedef struct {
    int width;
    int height;

    unsigned char *indices;
    unsigned int  *pixels;

} vad_image_t;


const unsigned int *vad_get_palette(void);

void vad_set_palette(const unsigned int *palette);
void vad_set_color(int index, unsigned int argb);
void vad_reload_pixels(vad_image_t *img);
void vad_free(vad_image_t *img);

void vad_draw(draw_ctx *ctx, const vad_image_t *img, int x, int y);
void vad_draw_scaled(
	draw_ctx *ctx, const vad_image_t *img,
    int x, int y, int w, int h
);
void vad_draw_ex(
	draw_ctx *ctx, const vad_image_t *img,
    int x, int y, int w, int h,
    int sat, int bright, int alpha
);

int vad_load(const char *path, vad_image_t *img);
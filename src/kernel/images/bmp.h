#ifndef BMP_H
#define BMP_H

#include <types.h>

typedef struct {
    i32 width;
    i32 height;
    u32 *data;
} bmp_image_t;

int bmp_load(const char *path, bmp_image_t *img);
void bmp_draw(bmp_image_t *img, u32 x, u32 y);
void bmp_free(bmp_image_t *img);

#endif

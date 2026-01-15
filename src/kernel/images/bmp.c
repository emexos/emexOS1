#include "bmp.h"
#include <memory/main.h>
#include <kernel/file_systems/vfs/vfs.h>
#include <kernel/graph/graphics.h>
#include <kernel/mem/klime/klime.h>

typedef struct {
    u16 type;
    u32 size;
    u16 reserved1;
    u16 reserved2;
    u32 offset;
} __attribute__((packed)) bmp_header_t;

typedef struct {
    u32 size;
    i32 width;
    i32 height;
    u16 planes;
    u16 bits_per_pixel;
    u32 compression;
    u32 image_size;
    i32 x_pixels_per_meter;
    i32 y_pixels_per_meter;
    u32 colors_used;
    u32 colors_important;
} __attribute__((packed)) bmp_info_t;

int bmp_load(const char *path, bmp_image_t *img) {
    int fd = fs_open(path, O_RDONLY);
    if (fd < 0) return -1;

    bmp_header_t header;
    fs_read(fd, &header, sizeof(header));

    if (header.type != 0x4D42) {
        fs_close(fd);
        return -1;
    }

    bmp_info_t info;
    fs_read(fd, &info, sizeof(info));

    img->width = info.width;
    img->height = info.height > 0 ? info.height : -info.height;

    // bytes per Pixal (3 für 24-bit)
    u32 bytes_per_pixel = info.bits_per_pixel / 8;

    // Row size  needs to be 4 bytes
    u32 row_size = ((info.width * bytes_per_pixel + 3) & ~3);
    u32 data_size = row_size * img->height;

    img->data = (u32 *)klime_create((klime_t *)fs_klime, img->width * img->height * sizeof(u32));
    if (!img->data) {
        fs_close(fd);
        return -1;
    }

    u8 *buffer = (u8 *)klime_create((klime_t *)fs_klime, data_size);
    if (!buffer) {
        klime_free((klime_t *)fs_klime, (u64 *)img->data);
        fs_close(fd);
        return -1;
    }

    // jmp to Pixel-Offset
    u32 current_pos = sizeof(bmp_header_t) + sizeof(bmp_info_t);
    if (header.offset > current_pos) {
        u8 dummy;
        for (u32 i = 0; i < (header.offset - current_pos); i++) {
            fs_read(fd, &dummy, 1);
        }
    }

    fs_read(fd, buffer, data_size);
    fs_close(fd);

    int flip = info.height > 0;

    for (i32 y = 0; y < img->height; y++) {
        i32 src_y = flip ? (img->height - 1 - y) : y;
        u8 *row = buffer + (src_y * row_size);

        for (i32 x = 0; x < img->width; x++) {
            // 24-Bit BMP: B-G-R format
            u8 b = row[x * bytes_per_pixel + 0];
            u8 g = row[x * bytes_per_pixel + 1];
            u8 r = row[x * bytes_per_pixel + 2];

            // convert to ARGB
            img->data[y * img->width + x] = 0xFF000000 | (r << 16) | (g << 8) | b;
        }
    }

    klime_free((klime_t *)fs_klime, (u64 *)buffer);
    return 0;
}

void bmp_draw(bmp_image_t *img, u32 x, u32 y) {
    if (!img || !img->data) return;

    for (i32 dy = 0; dy < img->height; dy++) {
        for (i32 dx = 0; dx < img->width; dx++) {
            putpixel(x + dx, y + dy, img->data[dy * img->width + dx]);
        }
    }
}

void bmp_free(bmp_image_t *img) {
    if (img && img->data) {
        klime_free((klime_t *)fs_klime, (u64 *)img->data);
        img->data = NULL;
    }
}

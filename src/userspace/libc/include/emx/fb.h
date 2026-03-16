#pragma once

// ioctl request codes for /dev/fb0
#define FBIOGET_VSCREENINFO 0x4600
#define FBIOPUT_VSCREENINFO 0x4601
#define FBIOGET_FSCREENINFO 0x4602
#define FBIO_RESET_POS      0x4603
#define FBIO_READ_RECT      0x4610
#define FBIO_BLIT           0x4611

typedef struct {
    unsigned int xres;
    unsigned int yres;
    unsigned int xres_virtual;
    unsigned int yres_virtual;
    unsigned int xoffset;
    unsigned int yoffset;
    unsigned int bits_per_pixel;
    unsigned int grayscale;
    unsigned int red_offset;
    unsigned int red_length;
    unsigned int green_offset;
    unsigned int green_length;
    unsigned int blue_offset;
    unsigned int blue_length;
    unsigned int transp_offset;
    unsigned int transp_length;
} fb_var_screeninfo;

typedef struct {
    char id[16];
    unsigned long smem_start; // physical address of framebuffer
    unsigned int smem_len;    // size in bytes
    unsigned int type;
    unsigned int visual;
    unsigned int line_length; // pitch in bytes
} fb_fix_screeninfo;

// used for both FBIO_READ_RECT and FBIO_BLIT
typedef struct {
    unsigned int  x, y, w, h;
    unsigned int *pixels;
} fb_rect_t;
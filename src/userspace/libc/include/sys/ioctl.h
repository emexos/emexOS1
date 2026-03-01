#pragma once
#include <sys/types.h>

// ioctl request codes for /dev/fb0
#define FBIOGET_VSCREENINFO 0x4600
#define FBIOPUT_VSCREENINFO 0x4601
#define FBIOGET_FSCREENINFO 0x4602
//#define FBPITCH



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
    unsigned int line_length; // pitch
} fb_fix_screeninfo;

// return:
//  0 == success
// -1 == error
int ioctl(int fd, int request, void *arg);
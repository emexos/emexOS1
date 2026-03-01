#ifndef DEVICE_FB0_H
#define DEVICE_FB0_H

#include <kernel/module/module.h>
#include <types.h>

#define FBN "emexfb0"

// fb_var_screeninfo
typedef struct {
    u32 xres;
    u32 yres;
    u32 xres_virtual;
    u32 yres_virtual;
    u32 xoffset;
    u32 yoffset;
    u32 bits_per_pixel;
    u32 grayscale;
    u32 red_offset;
    u32 red_length;
    u32 green_offset;
    u32 green_length;
    u32 blue_offset;
    u32 blue_length;
    u32 transp_offset;
    u32 transp_length;
} fb_var_screeninfo_t;
typedef struct {
    char id[16];
    u64 smem_start;
    u32 smem_len;
    u32 type;
    u32 visual;
    u32 line_length;   // pitch
} fb_fix_screeninfo_t;

// /dev/fb0 request codes
#define FBIOGET_VSCREENINFO  0x4600
#define FBIOPUT_VSCREENINFO  0x4601
#define FBIOGET_FSCREENINFO  0x4602

// /dev/fb0 module
extern driver_module fb0_module;



int fb0_ioctl(int request, void *arg);

#endif
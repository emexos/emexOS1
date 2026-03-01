#include "fb0.h"

#include <kernel/module/module.h>
#include <kernel/graph/graphics.h>
#include <kernel/communication/serial.h>
#include <string/string.h>
#include <theme/doccr.h>
#include <types.h>

#include <drivers/drivers.h>

static int fb0_mod_init(void) {
    log("[FB0]", "init /dev/fb0\n", d);
    return 0;
}

static void fb0_mod_fini(void) {}

static void *fb0_open(const char *path) {
    (void)path;
    return (void *)1; // dummy handle
}

static int fb0_read(void *handle, void *buf, size_t count) {
    (void)handle;
    u32 *fb = get_framebuffer();
    u32 w = get_fb_width();
    u32 h = get_fb_height();
    u32 pitch = get_fb_pitch();

    if (!fb) return -1;

    size_t fb_size = (size_t)pitch * h;
    if (count > fb_size) count = fb_size;

    memcpy(buf, (u8 *)fb, count);
    return (int)count;
}

static int fb0_write(void *handle, const void *buf, size_t count) {
    (void)handle;
    u32 *fb = get_framebuffer();
    u32 pitch = get_fb_pitch();
    u32 h = get_fb_height();

    if (!fb) return -1;

    size_t fb_size = (size_t)pitch * h;
    if (count > fb_size) count = fb_size;

    memcpy((u8 *)fb, buf, count);
    return (int)count;
}

int fb0_ioctl(int request, void *arg) {
    if (!arg) return -1;

    u32 w = get_fb_width();
    u32 h = get_fb_height();
    u32 pitch = get_fb_pitch();
    u32 *fb = get_framebuffer();

    switch (request) {
        case FBIOGET_VSCREENINFO: {
            fb_var_screeninfo_t *info = (fb_var_screeninfo_t *)arg;
            info->xres = w;
            info->yres = h;
            info->xres_virtual = w;
            info->yres_virtual = h;
            info->xoffset = 0;
            info->yoffset = 0;
            info->bits_per_pixel = 32;
            info->grayscale = 0;
            // ARGB8888: blue at 0, green at 8, red at 16
            info->blue_offset = 0;
            info->blue_length = 8;
            info->green_offset = 8;
            info->green_length = 8;
            info->red_offset = 16;
            info->red_length = 8;
            info->transp_offset = 24;
            info->transp_length = 8;
            return 0;
        }
        case FBIOGET_FSCREENINFO: {
            fb_fix_screeninfo_t *fix = (fb_fix_screeninfo_t *)arg;
            str_copy(fix->id, FBN);
            fix->smem_start = (u64)fb;
            fix->smem_len = pitch * h;
            fix->type = 0; // FB_TYPE_PACKED_PIXELS
            fix->visual = 2; // FB_VISUAL_TRUECOLOR
            fix->line_length = pitch;
            return 0;
        }
        default:
            return -1;
    }
}

driver_module fb0_module = {
    .name    = FB0NAME,
    .mount   = FB0PATH,
    .version = FB0UNIVERSAL,
    .init    = fb0_mod_init,
    .fini    = fb0_mod_fini,
    .open    = fb0_open,
    .read    = fb0_read,
    .write   = fb0_write,
};
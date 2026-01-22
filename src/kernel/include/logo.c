#include "logo.h"
#include "reqs.h"
#include <kernel/graph/graphics.h>
#include <kernel/file_systems/vfs/vfs.h>
#include <memory/main.h>
#include <string/string.h>
#include <theme/stdclrs.h>
#include <kernel/graph/theme.h>
#include <theme/doccr.h>
#include <kernel/mem/klime/klime.h>
#include <kernel/images/bmp.h>

// input data from logo
u8 *logo_data = NULL;
u32 logo_width = LOGO_WIDTH;
u32 logo_height = LOGO_HEIGHT;

void logo_init(void) {
    BOOTUP_PRINT("[LOGO] ", GFX_GRAY_70);

    logo_load_from_fs("/boot/ui/assets/bootlogo.bin");

    //bmp_load("/images/logo.bmp", bmp_image_t *img);

    //bmp_draw(&img, x, y);

    //bmp_free(&img);
}

int logo_load_from_fs(const char *path)
{
    int fd = fs_open(path, O_RDONLY);
    if (fd < 0) {
        BOOTUP_PRINT("cannot open: ", red());
        BOOTUP_PRINT(path, white());
        BOOTUP_PRINT("\n", white());
        return -1;
    }

    u64 expected_size = LOGO_WIDTH * LOGO_HEIGHT;
    logo_data = (u8*)klime_create((klime_t*)fs_klime, expected_size);

    if (!logo_data) {
        BOOTUP_PRINT("cannot allocate memory\n", red());
        fs_close(fd);
        return -1;
    }
    ssize_t bytes_read = fs_read(fd, logo_data, expected_size);
    fs_close(fd);

    if (bytes_read <= 0) {
        BOOTUP_PRINT("cannot read logo\n", red());
        klime_free((klime_t*)fs_klime, (u64*)logo_data);
        logo_data = NULL;
        return -1;
    }

    BOOTUP_PRINT("loaded from: ", white());
    BOOTUP_PRINT(path, white());
    BOOTUP_PRINT(" with ", white());

    char buf[32];
    str_copy(buf, "");
    str_append_uint(buf, (u32)bytes_read);
    BOOTUP_PRINT(buf, white());
    BOOTUP_PRINT(" bytes\n", white());

    return 0;
}

void draw_logo(void) {
    u32 screen_w = get_fb_width();
    u32 screen_h = get_fb_height();

    u32 start_x = (screen_w - logo_width * LOGO_SCALE) / 2;
    u32 start_y = (screen_h - logo_height * LOGO_SCALE) / 2;

    u32 fg_color = GFX_WHITE;

    for (u32 y = 0; y < logo_height; y++) {
        for (u32 x = 0; x < logo_width; x++) {
            if (logo_data[y * logo_width + x]) {
                //  draw LOGO_SCALE x LOGO_SCALE
                for (u32 dy = 0; dy < LOGO_SCALE; dy++) {
                    for (u32 dx = 0; dx < LOGO_SCALE; dx++) {
                        putpixel(start_x + x * LOGO_SCALE + dx ,
                                 start_y + y * LOGO_SCALE + dy ,
                                 fg_color);
                    }
                }
            }
        }
    }
}

#include <kernel/console/console.h>
#include <kernel/images/bmp.h>
#include <drivers/ps2/keyboard/keyboard.h>

FHDR(cmd_view) {
    /*if (!s || *s == '\0') {
        print("usage: view <image.bmp>\n", GFX_RED);
        return;
    }

    bmp_image_t img;
    if (bmp_load(s, &img) != 0) {
        print("error: cannot load image\n", GFX_RED);
        return;
    }

    //clear(CONSOLESCREEN_BG_COLOR);

    // centers image
    u32 x = (get_fb_width() - img.width) / 2;
    u32 y = (get_fb_height() - img.height) / 2;

    bmp_draw(&img, x, y);
     banner_draw();

    //print("\nPress any key to continue...\n", GFX_GRAY_50);

    while (1) {
        if (keyboard_has_key()) {
            key_event_t event;
            if (keyboard_get_event(&event) && event.pressed) {
                break;
            }
        }
        __asm__ volatile("hlt");
    }

    bmp_free(&img);
    //clear(CONSOLESCREEN_BG_COLOR);
    banner_draw();*/
}

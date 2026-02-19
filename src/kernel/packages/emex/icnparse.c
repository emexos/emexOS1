#include "emex.h"

#include <kernel/data/images/bmp.h>
#include <kernel/communication/serial.h>

//
// icon loader
//

int emex_load_icon(const char *icon_path, bmp_image_t *icon)
{
    if (!icon_path || !icon) return EMEX_ERR_NOT_FOUND;

    int result = bmp_load(icon_path, icon);
    if (result != 0) {
        printf("[EMX-ICON] failed to load icon from %s\n", icon_path);
        return EMEX_ERR_NOT_FOUND;
    }

    printf("[EMX-ICON] loaded icon (%dx%d)\n", icon->width, icon->height);
    return EMEX_OK;
}

void emex_draw_icon(bmp_image_t *icon, u32 x, u32 y) {
    if (!icon || !icon->data) {
        printf("[EMX-ICON] tried to draw a null icon, skipping\n");
        return;
    }

    bmp_draw(icon, x, y);
}

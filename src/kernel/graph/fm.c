#include "fm.h"
#include <ui/fonts.h>
#include <kernel/communication/serial.h>
#include <theme/stdclrs.h>
#include <kernel/graph/theme.h>
#include <theme/doccr.h>
static font_type_t current_font_type = FONT_8X8_BOLD; // default font
static const font_t *current_font = NULL;

void fm_init(void) {
    BOOTUP_PRINTBS("[FM] ", GFX_GRAY_70);
    BOOTUP_PRINTBS("start font manager\n", white());

    // default font is DOS font
    current_font_type = FONT_8X8_BOLD;
    current_font = &font_registry[FONT_8X8_BOLD];

    BOOTUP_PRINTBS("[FM] ", GFX_GRAY_70);
    BOOTUP_PRINTBS("default: ", white());
    BOOTUP_PRINTBS(current_font->name, GFX_ST_CYAN);
    BOOTUP_PRINTBS("\n", white());
}

int f_setcontext(font_type_t font_type)
{
    if (font_type >= FONT_COUNT) {
        BOOTUP_PRINT("[FM] ", GFX_GRAY_70);
        BOOTUP_PRINT("ERROR: invalid font type\n", red());
        return -1;
    }

    current_font_type = font_type;
    current_font = &font_registry[font_type];

    BOOTUP_PRINT("[FM] ", GFX_GRAY_70);
    BOOTUP_PRINT("switched to: ", white());
    BOOTUP_PRINT(current_font->name, GFX_ST_CYAN);
    BOOTUP_PRINT("\n", white());

    return 0;
}

font_type_t fm_get_current_font(void) {
    return current_font_type;
}

const u8* fm_get_glyph(u8 character) {
    if (!current_font || !current_font->data) return NULL;
    if (current_font->char_height == 8 && current_font->char_width == 8) {
        const u8 (*font_data_8)[256][8] = current_font->data;
        return (*font_data_8)[character];
    }
    // 8x16 fonts
    else if (current_font->char_height == 16 && current_font->char_width == 8) {
        const u8 (*font_data_16)[256][16] = current_font->data;
        return (*font_data_16)[character];
    }

    return NULL;
}
const u16* fm_get_glyph_16(u8 character) {
    if (!current_font || !current_font->data) return NULL;

    // 32 fonts
    if (current_font->char_width == 16 && current_font->char_height == 32) {
        const u16 (*font_data_16x32)[256][32] = current_font->data;
        return (*font_data_16x32)[character];
    }

    return NULL;
}
u32 fm_get_char_width(void) {
    if (!current_font) return 8;
    return current_font->char_width;
}
u32 fm_get_char_height(void) {
    if (!current_font) return 8;
    return current_font->char_height;
}
const char* fm_get_font_name(void) {return current_font->name;}

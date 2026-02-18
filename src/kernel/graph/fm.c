#include "fm.h"
#include <ui/fonts.h>
#include <kernel/communication/serial.h>
#include <theme/stdclrs.h>
#include <kernel/graph/theme.h>
#include <theme/doccr.h>
static font_type_t current_font_type = FONT_8X8_BOLD; // default font
static const font_t *current_font = NULL;

static u32 unicode_to_glyph_index(u32 cp)
{
    if (cp == 0x00A0) return (u32)' '; // NBSP

    // German umlauts/custom symbols in dedicated slots to avoid CP1251 conflicts.
    if (cp == 0x00E4) return 0x80; // ä
    if (cp == 0x00F6) return 0x81; // ö
    if (cp == 0x00FC) return 0x82; // ü
    if (cp == 0x00DF) return 0x83; // ß

    // Current glyph atlas is 256-entry, so only Latin-1 can map directly.
    if (cp <= 0x00FF) return cp;

    // Cyrillic mapped to CP1251 slots:
    // Ё=0xA8, ё=0xB8, А..Я=0xC0..0xDF, а..я=0xE0..0xFF (Russian Lang)
    if (cp == 0x0401) return 0xA8; // Ё
    if (cp == 0x0451) return 0xB8; // ё
    if (cp >= 0x0410 && cp <= 0x042F) return 0xC0 + (cp - 0x0410); // А..Я
    if (cp >= 0x0430 && cp <= 0x044F) return 0xE0 + (cp - 0x0430); // а..я

    return (u32)'?';
}

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

const u8* fm_get_glyph_cp(u32 codepoint) {
    if (!current_font || !current_font->data) return NULL;
    u32 glyph_index = unicode_to_glyph_index(codepoint);
    if (glyph_index > 0xFF) return NULL;

    if (current_font->char_height != 8 || current_font->char_width != 8) return NULL;
    const u8 (*font_data_8)[256][8] = current_font->data;
    return (*font_data_8)[glyph_index];
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

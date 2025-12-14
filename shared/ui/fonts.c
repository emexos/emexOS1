#include "fonts.h"
#include "fonts/font_8x8.h"
#include "fonts/font_8x8_dos.h"

// font registery
const font_t font_registry[FONT_COUNT] = {
    [FONT_8X8_DOS] = {
        .name = "8x8_DOS",
        .data = (const u8 (*)[132][8])&font_8x8_dos,
        .char_width = 8,
        .char_height = 8
    },
    [FONT_8X8] = {
        .name = "8x8",
        .data = (const u8 (*)[132][8])&font_8x8,
        .char_width = 8,
        .char_height = 8
    }
};

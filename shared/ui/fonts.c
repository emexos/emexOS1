#include "fonts.h"
#include "fonts/font_8x8.h"
#include "fonts/font_8x8_dos.h"
#include "fonts/font_8x16.h"

// font registery
const font_t font_registry[FONT_COUNT] = {
    [FONT_8X8_DOS] = {
        .name = "8x8_DOS",
        .data = font_8x8_dos,
        .char_width = 8,
        .char_height = 8
    },
    [FONT_8X16] = {
        .name = "8x16",
        .data = font_8x16,
        .char_width = 8,
        .char_height = 16
    },
    [FONT_8X8] = {
        .name = "8x8",
        .data = font_8x8,
        .char_width = 8,
        .char_height = 8
    }
};

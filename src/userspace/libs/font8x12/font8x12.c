#include "font8x12.h"
#include "data.h" // raw font data

uint16_t font8x12_glyph(unsigned char c, int row)
{
    return font_8x12[c & 0x7F][row];
}
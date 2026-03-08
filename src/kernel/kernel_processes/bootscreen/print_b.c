#include "print.h"
#include <kernel/graph/graphics.h>
#include <kernel/communication/serial.h>
#include <ui/fonts/font_8x8_bold.h>

static void putchar_bootstrap_at(char c, u32 x, u32 y, u32 color)
{
    const u8 *glyph = font_8x8_bold[(u8)c];
    u32 pitch_dwords = fb_pitch / 4;
    // bootstrap phase uses black background
    u32 bg_color = 0x00000000;

    for (int dy = 0; dy < 8; dy++)
    {
        u8 row = glyph[dy];
        // draw font_scale scanlines per glyph row
        for (u32 sy = 0; sy < font_scale; sy++) {
            u32 *fb_row = framebuffer + (y + dy * font_scale + sy) * pitch_dwords + x;
            for (int dx = 0; dx < 8; dx++) {
                // draw both fg and bg in single pass
                u32 pixel_color = (row & (1 << (7 - dx))) ? color : bg_color;
                for (u32 sx = 0; sx < font_scale; sx++) {
                    fb_row[dx * font_scale + sx] = pixel_color;
                }
            }
        }
    }
}

void putchar_bootstrap(char c, u32 color)
{
    u32 char_width = 8 * font_scale;
    u32 char_height = 8 * font_scale;
    u32 char_spacing = char_width;
    u32 line_height = char_height + 2 * font_scale;

    if (c == '\n')
    {
        cursor_x = 0; // past was 20
        cursor_y += line_height;
        return;
    }

    // check if we need to wrap to next line
    if (cursor_x + char_width >= fb_width)
    {
        cursor_x = 0; // past was 20
        cursor_y += line_height;
    }

    putchar_bootstrap_at(c, cursor_x, cursor_y, color);
    cursor_x += char_spacing;
}

void printbs(const char *str, u32 color)
{
    for (size_t i = 0; str[i]; i++)
    {
        putchar_bootstrap(str[i], color);
    }
    printf("%s", str); // prints everything from the os terminal to the host-terminal
}

#include "print.h"
#include <kernel/graph/graphics.h>
#include <kernel/communication/serial.h>
#include <ui/fonts/font_8x8_dos.h>
#include <kernel/console/graph/dos.h>

static void putchar_bootstrap_at(char c, u32 x, u32 y, u32 color)
{
    const u8 *glyph = font_8x8_dos[(u8)c];

    for (int dy = 0; dy < 8; dy++)
    {
        u8 row = glyph[dy];
        for (int dx = 0; dx < 8; dx++)
        {
            if (row & (1 << (7 - dx)))
            {
                // Draw scaled pixel
                for (u32 sy = 0; sy < font_scale; sy++) {
                    for (u32 sx = 0; sx < font_scale; sx++) {
                        putpixel(x + dx * font_scale + sx, y + dy * font_scale + sy, color);
                    }
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

    // Check if we need to wrap to next line
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

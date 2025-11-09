#include "print.h"
#include <klib/graphics/graphics.h>
#include <fonts/font_8x8.h>

static void putchar_at(char c, u32 x, u32 y, u32 color)
{
    const u8 *glyph = font_8x8[(u8)c];

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

void putchar(char c, u32 color)
{
    u32 char_width = 8 * font_scale - 1 * font_scale;
    u32 char_height = 8 * font_scale;
    u32 char_spacing = char_width + font_scale;
    u32 line_height = char_height + 2 * font_scale;

    if (c == '\n')
    {
        cursor_x = 20;
        cursor_y += line_height;

        // Check if we need to scroll
        if (cursor_y + char_height >= fb_height) {
            scroll_up(line_height);
            cursor_y = fb_height - char_height - 10;
        }
        return;
    }

    // Check if we need to wrap to next line
    if (cursor_x + char_width >= fb_width)
    {
        cursor_x = 20;
        cursor_y += line_height;

        // Check if we need to scroll after wrapping
        if (cursor_y + char_height >= fb_height) {
            scroll_up(line_height);
            cursor_y = fb_height - char_height - 10;
        }
    }

    // Check if we're at bottom before printing
    if (cursor_y + char_height >= fb_height)
    {
        scroll_up(line_height);
        cursor_y = fb_height - char_height - 10;
    }

    putchar_at(c, cursor_x, cursor_y, color);
    cursor_x += char_spacing;
}

void string(const char *str, u32 color)
{
    for (size_t i = 0; str[i]; i++)
    {
        putchar(str[i], color);
    }
}

void IntToString(int value, char *buffer)
{
    char temp[11];
    int i = 0;
    int isNegative = 0;

    if (value < 0)
    {
        isNegative = 1;
        value = -value;
    }

    do
    {
        temp[i++] = (value % 10) + '0';
        value /= 10;
    } while (value > 0);

    if (isNegative)
    {
        temp[i++] = '-';
    }

    temp[i] = '\0';

    for (int j = 0; j < i; j++)
    {
        buffer[j] = temp[i - j - 1];
    }

    buffer[i] = '\0';
}

void printInt(int value, u32 color)
{
    char buffer[12];
    IntToString(value, buffer);
    string(buffer, color);
}

void print(const char *str, u32 color)
{
    string(str, color);
    //putchar('\n', color);
}

void reset_cursor(void)
{
    cursor_x = 0;
    cursor_y = 0;
}

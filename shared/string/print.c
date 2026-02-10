#include "print.h"
#include <kernel/graph/graphics.h>
#include <kernel/graph/fm.h>
#include <kernel/communication/serial.h>
//#include <ui/fonts/font_8x8.h>

#include <kernel/console/graph/dos.h>

static void putchar_at(char c, u32 x, u32 y, u32 color)
{
    u32 char_width = fm_get_char_width();
    u32 char_height = fm_get_char_height();

    // 16 fonts
    if (char_width == 16 && char_height == 32) {
        const u16 *glyph = fm_get_glyph_16((u8)c);
        if (!glyph) return;
        for (u32 dy = 0; dy < char_height; dy++)
        {
            u16 row = glyph[dy];
            for (u32 dx = 0; dx < char_width; dx++)
            {
                if (row & (1 << (15 - dx)))
                {
                    for (u32 sy = 0; sy < font_scale; sy++) {
                        for (u32 sx = 0; sx < font_scale; sx++) {
                            putpixel(x + dx * font_scale + sx, y + dy * font_scale + sy, color);
                        }
                    }
                }
            }
        }
    }
    else { // 8 fonts
        const u8 *glyph = fm_get_glyph((u8)c);
        if (!glyph) return;
        for (u32 dy = 0; dy < char_height; dy++)
        {
            u8 row = glyph[dy];
            for (u32 dx = 0; dx < char_width; dx++)
            {
                if (row & (1 << (7 - dx)))
                {
                    for (u32 sy = 0; sy < font_scale; sy++) {
                        for (u32 sx = 0; sx < font_scale; sx++) {
                            putpixel(x + dx * font_scale + sx, y + dy * font_scale + sy, color);
                        }
                    }
                }
            }
        }
    }
}

/*static void aware_scroll_byprint(void)
{
    u32 char_height = fm_get_char_height() * font_scale;
    u32 fb_h = get_fb_height();

    // Check if cursor is out of screen
    if (cursor_y + char_height > fb_h) {
        u32 line_height = char_height + 2 * font_scale;

        // Use scroll_up from graphics.c which is already theme-aware
        scroll_up(line_height);

        // Move cursor up
        cursor_y -= line_height;
        if (cursor_y < 0) {
            cursor_y = 0;
        }
    }
}*/

void putchar(char c, u32 color)
{
    u32 char_width = fm_get_char_width() * font_scale;
    u32 char_height = fm_get_char_height() * font_scale;
    u32 char_spacing = char_width;
    u32 line_height = char_height + 2 * font_scale;

    if (c == '\n')
    {
        cursor_x = 0; // past was 20
        cursor_y += line_height;

        // use console window scroll check
        console_window_check_scroll();
        //aware_scroll_byprint();
        return;
    }

    // Check if we need to wrap to next line
    if (cursor_x + char_width >= fb_width)
    {
        cursor_x = 0; // past was 20
        cursor_y += line_height;
        console_window_check_scroll();
    }

    console_window_check_scroll();

    putchar_at(c, cursor_x, cursor_y, color);
    cursor_x += char_spacing;
}

void string(const char *str, u32 color)
{
    for (size_t i = 0; str[i]; i++)
    {
        putchar(str[i], color);
    }
    printf("%s", str); // prints everything from the os terminal to the host-terminal
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

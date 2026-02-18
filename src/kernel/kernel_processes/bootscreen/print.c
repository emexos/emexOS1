#include "print.h"
#include <kernel/graph/graphics.h>
#include <kernel/kernel_processes/fm/fm.h>
#include <kernel/communication/serial.h>
//#include <ui/fonts/font_8x8.h>

#include <kernel/console/graph/dos.h>

// made by Dexoron Feb. 18, 2026
// Decode one UTF-8 sequence and return pointer to next byte.
// On invalid data, returns '?' and advances by one byte.
static const char *utf8_next_codepoint(const char *p, u32 *codepoint)
{
    if (!p || !codepoint) return p;

    unsigned char b0 = (unsigned char)p[0];
    if (b0 == 0) {
        *codepoint = 0;
        return p;
    }

    // 1-byte ASCII: 0xxxxxxx
    if ((b0 & 0x80) == 0) {
        *codepoint = b0;
        return p + 1;
    }

    // Continuation byte as lead is invalid.
    if ((b0 & 0xC0) == 0x80) {
        *codepoint = '?';
        return p + 1;
    }

    // 2-byte: 110xxxxx 10xxxxxx
    if ((b0 & 0xE0) == 0xC0) {
        unsigned char b1 = (unsigned char)p[1];
        if ((b1 & 0xC0) != 0x80) {
            *codepoint = '?';
            return p + 1;
        }

        u32 cp = ((u32)(b0 & 0x1F) << 6) | (u32)(b1 & 0x3F);
        // Overlong encoding check: must be >= 0x80
        if (cp < 0x80) {
            *codepoint = '?';
            return p + 1;
        }

        *codepoint = cp;
        return p + 2;
    }

    // 3-byte: 1110xxxx 10xxxxxx 10xxxxxx
    if ((b0 & 0xF0) == 0xE0) {
        unsigned char b1 = (unsigned char)p[1];
        unsigned char b2 = (unsigned char)p[2];
        if ((b1 & 0xC0) != 0x80 || (b2 & 0xC0) != 0x80) {
            *codepoint = '?';
            return p + 1;
        }

        u32 cp = ((u32)(b0 & 0x0F) << 12) |
                 ((u32)(b1 & 0x3F) << 6) |
                 (u32)(b2 & 0x3F);
        // Overlong and UTF-16 surrogate range checks
        if (cp < 0x800 || (cp >= 0xD800 && cp <= 0xDFFF)) {
            *codepoint = '?';
            return p + 1;
        }

        *codepoint = cp;
        return p + 3;
    }

    // 4-byte: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    if ((b0 & 0xF8) == 0xF0) {
        unsigned char b1 = (unsigned char)p[1];
        unsigned char b2 = (unsigned char)p[2];
        unsigned char b3 = (unsigned char)p[3];
        if ((b1 & 0xC0) != 0x80 || (b2 & 0xC0) != 0x80 || (b3 & 0xC0) != 0x80) {
            *codepoint = '?';
            return p + 1;
        }

        u32 cp = ((u32)(b0 & 0x07) << 18) |
                 ((u32)(b1 & 0x3F) << 12) |
                 ((u32)(b2 & 0x3F) << 6) |
                 (u32)(b3 & 0x3F);
        // Valid Unicode scalar range: U+10000..U+10FFFF
        if (cp < 0x10000 || cp > 0x10FFFF) {
            *codepoint = '?';
            return p + 1;
        }

        *codepoint = cp;
        return p + 4;
    }

    *codepoint = '?';
    return p + 1;
}

static void putchar_at(u32 codepoint, u32 x, u32 y, u32 color)
{
    u32 char_width = fm_get_char_width();
    u32 char_height = fm_get_char_height();
    u32 row_bytes = fm_get_glyph_row_bytes();
    u32 lsb_left = fm_get_glyph_lsb_left();

    const u8 *glyph = fm_get_glyph_cp(codepoint);
    if (!glyph) return;
    for (u32 dy = 0; dy < char_height; dy++)
    {
        u32 row = 0;
        const u8 *row_ptr = glyph + (dy * row_bytes);
        if (row_bytes == 1) {
            row = row_ptr[0];
        } else if (row_bytes == 2) {
            row = (u32)((u16)row_ptr[0] | ((u16)row_ptr[1] << 8));
        } else if (row_bytes == 4) {
            row = (u32)row_ptr[0] |
                  ((u32)row_ptr[1] << 8) |
                  ((u32)row_ptr[2] << 16) |
                  ((u32)row_ptr[3] << 24);
        } else {
            return;
        }

        for (u32 dx = 0; dx < char_width; dx++)
        {
            u32 bit_index = lsb_left ? dx : ((char_width - 1u) - dx);
            if (row & (1u << bit_index))
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

static void putcodepoint(u32 codepoint, u32 color)
{
    u32 char_width = fm_get_char_width() * font_scale;
    u32 char_height = fm_get_char_height() * font_scale;
    u32 char_spacing = char_width;
    u32 line_height = char_height + 2 * font_scale;

    if (codepoint == '\n')
    {
        cursor_x = 0;
        cursor_y += line_height;
        console_window_check_scroll();
        return;
    }

    if (cursor_x + char_width >= fb_width)
    {
        cursor_x = 0;
        cursor_y += line_height;
        console_window_check_scroll();
    }

    console_window_check_scroll();

    putchar_at(codepoint, cursor_x, cursor_y, color);
    cursor_x += char_spacing;
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
    putcodepoint((u32)(unsigned char)c, color);
}

void string(const char *str, u32 color)
{
    const char *p = str;
    while (p && *p) {
        u32 cp = 0;
        p = utf8_next_codepoint(p, &cp);
        if (cp == 0) break;
        putcodepoint(cp, color);
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

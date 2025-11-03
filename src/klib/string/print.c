#include "../../../shared/types.h"
#include "../../../shared/fonts/font.h"
#include "print.h"
#include "../string/string.h"
#include "../graphics/graphics.h"

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
                putpixel(x + dx, y + dy, color);
            }
        }
    }
}

void putchar(char c, u32 color)
{
    if (c == '\n')
    {
        cursor_x = 20;
        cursor_y += 10;
        return;
    }

    if (cursor_x + 8 >= fb_width)
    {
        cursor_x = 20;
        cursor_y += 10;
    }

    if (cursor_y + 8 >= fb_height)
    {
        cursor_y = 10;
    }

    putchar_at(c, cursor_x, cursor_y, color);
    cursor_x += 9;
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
    cursor_x = 10;
    cursor_y = 10;
}

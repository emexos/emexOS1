#include "../../shared/types.h"
#include "graphics.h"
//#include "draw.h"

void draw_rect(u32 x, u32 y, u32 width, u32 height, u32 color)
{
    for (u32 dy = 0; dy < height; dy++)
    {
        for (u32 dx = 0; dx < width; dx++)
        {
            putpixel(x + dx, y + dy, color);
        }
    }
}

void draw_circle(u32 cx, u32 cy, u32 radius, u32 color)
{
    for (u32 y = 0; y <= radius; y++)
    {
        for (u32 x = 0; x <= radius; x++)
        {
            if (x * x + y * y <= radius * radius)
            {
                putpixel(cx + x, cy + y, color);
                putpixel(cx - x, cy + y, color);
                putpixel(cx + x, cy - y, color);
                putpixel(cx - x, cy - y, color);
            }
        }
    }
}

void draw_line(u32 x0, u32 y0, u32 x1, u32 y1, u32 color)
{
    int dx = x1 - x0;
    int dy = y1 - y0;

    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;

    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;

    while (1)
    {
        putpixel(x0, y0, color);

        if (x0 == x1 && y0 == y1)
            break;

        int e2 = 2 * err;
        if (e2 > -dy)
        {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

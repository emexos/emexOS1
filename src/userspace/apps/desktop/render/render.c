#include "render.h"
#include "../compositor/comp.h"
#include "../config/cfg.h"
#include "../win/win.h"
#include "../../../libs/font8x12/font8x12.h"

#define ROW_MAX 4096
static unsigned int row_buf[ROW_MAX];


static int slen(const char *s) { int n = 0; while (s[n]) n++; return n; }

static unsigned int stripe(int y, int focused)
{
    if (focused) return (y & 1) ? DT_BLACK : DT_WHITE;
    return (y & 1) ? DT_SHADOW : DT_LIGHT;
}

static void buf_char(int bx, char c, unsigned int fg, unsigned int bg, int frow)
{
    unsigned int bits = font8x12_glyph((unsigned char)c & 0x7Fu, frow);

    for (int col = 0; col < DT_FW; col++)
    {
        int dx = bx + col;
        if (dx >= 0 && dx < ROW_MAX) row_buf[dx] = (bits & (1u << col)) ? fg : bg;
    }
}

static void buf_str(
	int bx, const char *s,
    unsigned int fg, unsigned int bg, int frow
) {
    while (*s) { buf_char(bx, *s++, fg, bg, frow); bx += DT_FW; }
}

static void flush_row(int x, int y, int w) {
    comp_put_row(x, y, row_buf, w);
}

static void hline_comp(int x, int y, int w, unsigned int c)
{
    for (int dx = 0; dx < w && dx < ROW_MAX; dx++) row_buf[dx] = c;
    comp_put_row(x, y, row_buf, w);
}
// to the backbuffer
static void side_borders_comp(int wx, int y, int ww)
{
    comp_set(wx,          y, DT_BLACK);
    comp_set(wx + 1,      y, DT_BLACK);
    comp_set(wx + ww - 2, y, DT_BLACK);
    comp_set(wx + ww - 1, y, DT_BLACK);
}


//pubs
void render_win(dt_win_t *w)
{
    int wx = w->x, wy = w->y;
    int ww = w->w, wh = w->h;

    int focused = w->focused;

    unsigned int style = w->style;

    int h = comp_h();
    int cw = comp_w();
    (void)cw;

    if (style & DT_POPUP)
    {
    	// content will be untouched
        hline_comp(wx, wy,          ww, DT_BLACK);
        hline_comp(wx, wy + wh - 1, ww, DT_BLACK);
        for (int dy = 1; dy < wh - 1; dy++) {
            int ay = wy + dy;
            if (ay < 0 || ay >= h) continue;
            comp_set(wx,          ay, DT_BLACK);
            comp_set(wx + ww - 1, ay, DT_BLACK);
        }
        return;
    }

    int has_title = !(style & DT_NOTITLE);
    int tw = slen(w->title) * DT_FW;
    int tx = (ww - tw) / 2;
    int sl = tx - 4;
    int sr = tx + tw + 4;

    unsigned int title_bg = has_title
        ? (focused ? DT_TITLE_ACT : DT_TITLE_INA)
        : DT_FACE
    ;

    for (int dy = 0; dy < wh; dy++)
    {
        int ay = wy + dy;
        if (ay < 0 || ay >= h) continue;
        if (dy < 2 || dy >= wh - 2) {
            hline_comp(wx, ay, ww, DT_BLACK);
            continue;
        }
        if (has_title && dy >= 2 && dy < DT_TITLE_H)
        {
        	int frow = dy - DT_TITLE_PB; // title text

            // fill titlebar bg
            for (int dx = 2; dx < ww - 2 && dx < ROW_MAX; dx++)
            {
                row_buf[dx] = (dx < sl || dx >= sr)
                    ? stripe(ay, focused)
                    : title_bg;
            }
            // left, right border pixels
            row_buf[0] = row_buf[1] = DT_BLACK;

            if (ww - 2 < ROW_MAX) row_buf[ww - 2] = row_buf[ww - 1] = DT_BLACK;

            if (frow >= 0 && frow < DT_FH) buf_str(tx, w->title, DT_TITLE_TXT, title_bg, frow);

            // close button
            {
                int bx  = DT_CLOSE_X;
                int bry = dy - DT_CLOSE_Y;
                if (bry >= 0 && bry <= DT_CLOSE_SZ)
                {
                    if (bry == 0 || bry == DT_CLOSE_SZ)
                    {
                        for (
                        	int dx = bx; dx <= bx + DT_CLOSE_SZ && dx < ROW_MAX; dx++
                        ) row_buf[dx] = DT_BLACK;
                    } else
                    {
                        if (bx < ROW_MAX) row_buf[bx] = DT_BLACK;
                        if (bx + DT_CLOSE_SZ < ROW_MAX) row_buf[bx + DT_CLOSE_SZ] = DT_BLACK;
                        for (
                        	int dx = bx + 1; dx < bx + DT_CLOSE_SZ && dx < ROW_MAX; dx++
                        ) row_buf[dx] = DT_FACE;
                    }
                }
            }

            flush_row(wx, ay, ww);
            continue;
        }

        // separator line after the titlebar
        if (has_title && dy == DT_TITLE_H)
        {
            hline_comp(wx, ay, ww, DT_BLACK);
            continue;
        }

        // content + window border
        {
            int cs = has_title ? (DT_TITLE_H + 1) : 2;
            if (dy >= cs && dy < wh - 2) side_borders_comp(wx, ay, ww);
        }
    }
}

void render_all(void)
{
    int order[DT_WIN_MAX];
    int count = 0;

    for (int i = 0; i < DT_WIN_MAX; i++)
    {
        if (win_get(i)) order[count++] = i;
    }
    for (int i = 0; i < count - 1; i++)
    {
        for (int j = 0; j < count - 1 - i; j++)
        {
            dt_win_t *a = win_get(order[j]);
            dt_win_t *b = win_get(order[j + 1]);
            if (a && b && a->z > b->z)
            {
                int tmp = order[j];
                order[j] = order[j + 1];
                order[j + 1] = tmp;
            }
        }
    }

    for (int i = 0; i < count; i++)
    {
        dt_win_t *wn = win_get(order[i]);
        if (wn) render_win(wn);
    }
}
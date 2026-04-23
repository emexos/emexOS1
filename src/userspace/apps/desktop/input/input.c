#include "input.h"
#include "../win/win.h"

#include <unistd.h>
#include <emx/mouse.h>

static int g_last_btn = 0;
static int g_drag_idx = -1;
static int g_drag_ox = 0;
static int g_drag_oy = 0;

void input_init(void)
{
    g_last_btn = 0;
    g_drag_idx = -1;
}

void input_frame_begin(input_state_t *is)
{
    (void)is;
}

static int handle_one(mouse_event_t *ev, input_state_t *is)
{
    int mx = ev->abs_x;
    int my = ev->abs_y;
    int btn = ev->buttons & MOUSE_BTN_LEFT;
    int changed = 0;

    is->cx = mx;
    is->cy = my;

    if (btn && !g_last_btn)
    {
        int top_idx = -1, top_z = -1;
        for (int i = 0; i < DT_WIN_MAX; i++)
        {
            dt_win_t *wn = win_get(i);
            if (!wn) continue;
            if (win_hit(i, mx, my) && wn->z > top_z)
            {
                top_z = wn->z; top_idx = i;
            }
        }
        if (top_idx >= 0)
        {
            win_focus(top_idx);
            changed = 1;

            if (win_hit_close(top_idx, mx, my))
            {
                dt_win_t *wn = win_get(top_idx);
                if (wn) { win_remove(wn->pid); }
                //
            }else if (win_hit_title(top_idx, mx, my))
            {
                dt_win_t *wn = win_get(top_idx);
                if (wn && !(wn->style & DT_NOMOVE) && !(wn->style & DT_NOTITLE))
                {
                    g_drag_idx = top_idx;
                    g_drag_ox  = mx - wn->x;
                    g_drag_oy  = my - wn->y;
                }
            }
        }
    }

    if (!btn) g_drag_idx = -1;

    if (g_drag_idx >= 0) {
        int nx = mx - g_drag_ox;
        int ny = my - g_drag_oy;
        if (nx < 0) nx = 0;
        if (ny < 0) ny = 0;

        win_move(g_drag_idx, nx, ny);
        changed = 1;
    }

    g_last_btn = btn;
    return changed;
}

int input_drain(int mfd, input_state_t *is)
{
    mouse_event_t ev;
    int got = 0;

    while ((int)read(mfd, &ev, sizeof(ev)) == (int)sizeof(ev))
    {
        if (handle_one(&ev, is)) is->win_changed = 1;
        got = 1;
    }

    return got;
}
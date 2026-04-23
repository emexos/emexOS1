#include "win.h"
#include "../config/cfg.h"
#include <string.h>

// window creating
// the guilui (gui lib) should handle the window content

static dt_win_t wins[DT_WIN_MAX];
static int win_cnt = 0;
static int z_next  = 0;

static void scopy(char *dst, const char *src, int max)
{
    int i = 0;
    while (i < max - 1 && src[i]) { dst[i] = src[i]; i++; }
    dst[i] = '\0';
}

int win_add(
	pid_t pid, const char *title,
    int x, int y, int w, int h, unsigned int style
) {
    for (int i = 0; i < DT_WIN_MAX; i++)
    {
        if (!wins[i].valid)
        {
            wins[i].pid = pid;

            wins[i].x = x;
            wins[i].y = y;
            wins[i].w = w;
            wins[i].h = h;

            wins[i].style = style;
            wins[i].valid = 1;
            wins[i].focused = 0;
            wins[i].z = z_next++;

            scopy(wins[i].title, title, DT_TITLE_MAX);

            win_cnt++;

            return i;
        }

    }

    return -1;
}

void win_remove(pid_t pid)
{
    int i = win_find_pid(pid);
    if (i < 0) return;

    wins[i].valid = 0;
    win_cnt--;
}

void win_set_title(pid_t pid, const char *title)
{
    int i = win_find_pid(pid);
    if (i < 0) return;

    scopy(wins[i].title, title, DT_TITLE_MAX);
}

void win_move(int idx, int nx, int ny)
{
    if (idx < 0 || idx >= DT_WIN_MAX || !wins[idx].valid) return;
    wins[idx].x = nx;
    wins[idx].y = ny;
    // home stays at orig. pos.
}

void win_focus(int idx)
{
	// auto unfocuses the others then
    for (int i = 0; i < DT_WIN_MAX; i++) wins[i].focused = 0;
    if (idx >= 0 && idx < DT_WIN_MAX && wins[idx].valid)
    {
        wins[idx].focused = 1;
        wins[idx].z = z_next++;
    }
}

int win_find_pid(pid_t pid)
{
    for (int i = 0; i < DT_WIN_MAX; i++) if (wins[i].valid && wins[i].pid == pid) return i;
    return -1;
}

int win_hit(int idx, int mx, int my)
{
    dt_win_t *w = win_get(idx);
    if (!w) return 0;

    return
    	mx >= w->x && mx < w->x + w->w &&
        my >= w->y && my < w->y + w->h
    ;
}

int win_hit_title(int idx, int mx, int my)
{
    dt_win_t *w = win_get(idx);
    if (!w || (w->style & DT_POPUP) || (w->style & DT_NOTITLE)) return 0;

    return
    	mx >= w->x && mx < w->x + w->w &&
        my >= w->y && my < w->y + DT_TITLE_H
    ;
}

int win_hit_close(int idx, int mx, int my)
{
    dt_win_t *w = win_get(idx);
    if (!w || (w->style & DT_POPUP) || (w->style & DT_NOTITLE)) return 0;
    int bx = w->x + DT_CLOSE_X;
    int by = w->y + DT_CLOSE_Y;
    return
    	mx >= bx && mx < bx + DT_CLOSE_SZ &&
        my >= by && my < by + DT_CLOSE_SZ
    ;
}

dt_win_t *win_get(int idx)
{
    if (idx < 0 || idx >= DT_WIN_MAX || !wins[idx].valid) return 0;
    return &wins[idx];
}

int win_count(void)
{
	return win_cnt;
}
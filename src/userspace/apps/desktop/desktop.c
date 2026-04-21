#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>

#include "config/cfg.h"
#include "compositor/comp.h"
#include "bg/bg.h"
#include "win/win.h"
#include "render/render.h"
#include "cursor/cursor.h"
#include "input/input.h"
#include "cmd/cmd.h"

#include <emx/fb.h>


static int _slen(const char *s)
{
	int n = 0;
	while (s[n]) n++;
	return n;
}

static void _itoa(int v, char *out)
{
    char tmp[16];
    int i = 0;
    int neg = (v < 0);
    int j = 0;

    if (v == 0)
    {
    	out[0]='0';
     	out[1]='\0';
     	return;
    }

    if (neg) v = -v;
    while (v)
    {
    	tmp[i++] = '0' + v % 10; v /= 10;
    }
    if (neg) tmp[i++] = '-';
    while (i > 0) out[j++] = tmp[--i]; out[j] = '\0';
}

static void write_cursor_pos(int x, int y)
{
    char buf[32];
    char xs[12];
    char ys[12];
    int i = 0;
    int j = 0;
    int fd = open(DT_CURSOR, O_WRONLY | O_CREAT);

    _itoa(x, xs); _itoa(y, ys);

    while (xs[j]) buf[i++] = xs[j++];
    buf[i++] = ' ';
    j = 0;

    while (ys[j]) buf[i++] = ys[j++];

    buf[i++] = '\n';
    buf[i] = '\0';

    if (fd >= 0) { write(fd, buf, (unsigned)_slen(buf)); close(fd); }
}

static void current_content(
	dt_win_t *w,
    int *cx, int *cy, int *cw, int *ch
) {
    unsigned int s = w->style;
    if (s & DT_POPUP) {
        *cx = w->x + 1;
        *cy = w->y + 1;
        *cw = w->w - 2;
        *ch = w->h - 2;
    } else if (s & DT_NOTITLE) {
        *cx = w->x + DT_BORDER;
        *cy = w->y + DT_BORDER;
        *cw = w->w - DT_BORDER * 2;
        *ch = w->h - DT_BORDER * 2;
    } else {
        *cx = w->x + DT_BORDER;
        *cy = w->y + DT_TITLE_H + 1;
        *cw = w->w - DT_BORDER * 2;
        *ch = w->h - DT_TITLE_H - 1 - DT_BORDER;
    }
}

static void _old_full_rect(
	dt_win_t *w, int hcx, int hcy, int hcw, int hch,
    int *ox, int *oy, int *ow, int *oh
){
    unsigned int s = w->style;
    if (s & DT_POPUP) {
        *ox = hcx - 1;
        *oy = hcy - 1;
        *ow = hcw + 2;
        *oh = hch + 2;
    } else if (s & DT_NOTITLE) {
        *ox = hcx - DT_BORDER;
        *oy = hcy - DT_BORDER;
        *ow = hcw + DT_BORDER * 2;
        *oh = hch + DT_BORDER * 2;
    } else {
        *ox = hcx - DT_BORDER;
        *oy = hcy - DT_TITLE_H - 1;
        *ow = hcw + DT_BORDER * 2;
        *oh = hch + DT_TITLE_H + 1 + DT_BORDER;
    }
}

static void sync_home_to_current(void)
{
    for (int i = 0; i < DT_WIN_MAX; i++)
    {
        dt_win_t *w = win_get(i);
        if (!w) continue;

        int cx, cy, cw, ch;
        current_content(w, &cx, &cy, &cw, &ch);

        // if window hasn't moved, home == current, nothing to do
        if (cx == w->home_cx && cy == w->home_cy) continue;

        comp_copy_rect(
        	w->home_cx, w->home_cy,
            cx, cy,
        	w->home_cw, w->home_ch
        );

        int ox, oy, ow, oh;
        _old_full_rect(
        	w, w->home_cx, w->home_cy, w->home_cw, w->home_ch,
            &ox, &oy, &ow, &oh
        );
        int nox, noy, now_, noh;
        _old_full_rect(w, cx, cy, cw, ch, &nox, &noy, &now_, &noh);
        {
            int ix0 = ox > nox ? ox : nox;
            int iy0 = oy > noy ? oy : noy;
            int ix1 = (ox+ow)<(nox+now_) ? (ox+ow) : (nox+now_);
            int iy1 = (oy+oh)<(noy+noh) ? (oy+oh) : (noy+noh);
            if (ix0 >= ix1 || iy0 >= iy1) {
                bg_draw_rect(ox, oy, ow, oh);
            } else {
                if (oy < iy0)
                    bg_draw_rect(ox, oy, ow, iy0-oy);
                if (oy+oh > iy1)
                    bg_draw_rect(ox, iy1, ow, (oy+oh)-iy1);
                if (iy0 < iy1) {
                    if (ox < ix0)
                        bg_draw_rect(ox, iy0, ix0-ox, iy1-iy0);
                    if (ox+ow > ix1)
                        bg_draw_rect(ix1, iy0, (ox+ow)-ix1, iy1-iy0);
                }
            }
        }

        w->home_cx = cx;
        w->home_cy = cy;
        w->home_cw = cw;
        w->home_ch = ch;
    }
}

static void clear_old_frames(cmd_result_t *cr, drag_info_t *drag_prev)
{
    for (int i = 0; i < cr->count; i++)
    {
        drag_info_t *d = &cr->rects[i];

        if (d->valid) bg_draw_rect(d->wx, d->wy, d->ww, d->wh);
    }
    if (drag_prev && drag_prev->valid)
    {
        bg_draw_rect(
        	drag_prev->wx, drag_prev->wy,
            drag_prev->ww, drag_prev->wh
        );

        drag_prev->valid = 0;
    }
}


int main(void)
{
    mkdir(DT_DIR);

    int fd;
    fd = open(DT_CMD, O_WRONLY | O_CREAT); if (fd >= 0) close(fd);
    fd = open(DT_DIRTY, O_WRONLY | O_CREAT);
    if (fd >= 0) close(fd);

    int fb = open("/dev/fb0", O_RDWR);
    int mfd = open(MOUSE_DEV, O_RDONLY);

    if (fb < 0 || mfd < 0) return 1;

    fb_var_screeninfo vinfo;
    ioctl(fb, FBIOGET_VSCREENINFO, &vinfo);

    int scr_w = (int)vinfo.xres;
    int scr_h = (int)vinfo.yres;

    //TODO:
    // look for real display size
    if (scr_w <= 0) scr_w = 1024;
    if (scr_h <= 0) scr_h = 768;

    comp_init(fb, scr_w, scr_h);
    bg_init(scr_w, scr_h);
    cur_init(fb, scr_w, scr_h);
    input_init();

    bg_draw_full();
    comp_flush();

    input_state_t is;

    is.cx = scr_w / 2;
    is.cy = scr_h / 2;

    is.win_changed = 0;
    is.drag_prev.valid = 0;

    cur_draw_fb(is.cx, is.cy);
    write_cursor_pos(is.cx, is.cy);

    int poll_tick = 0;

    #define POLL_INTERVAL 4

    for (;;)
    {
        int need_full = 0;
        int need_cur  = 0;

        cmd_result_t cr;
        cr.count = 0; cr.win_changed = 0;

        poll_tick++;

        int app_dirty = 0;
        if (poll_tick >= POLL_INTERVAL)
        {
            poll_tick = 0;
            cmd_process(&cr);
            if (cr.win_changed) need_full = 1;
            app_dirty = cmd_check_dirty();
            if (app_dirty) need_full = 1;
        }

        input_frame_begin(&is);

        if (input_drain(mfd, &is))
        {
            need_cur = 1;
            write_cursor_pos(is.cx, is.cy);
        }
        if (is.win_changed)
        {
            need_full = 1;
            is.win_changed = 0;
        }

        if (need_full)
        {
            comp_capture();
            cur_undo_from_backbuf();
            sync_home_to_current();
            clear_old_frames(&cr, &is.drag_prev);
            render_all();
            cur_bake(is.cx, is.cy);
            comp_flush();
        } else if (need_cur)
        {
            cur_erase_fb();
            cur_draw_fb(is.cx, is.cy);
        }

        if (!need_full && !need_cur)
        {
            for (volatile int i = 0; i < 500; i++)
                __asm__ volatile("pause");
        }
    }
}
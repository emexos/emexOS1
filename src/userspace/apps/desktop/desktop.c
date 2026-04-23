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

    input_state_t is;

    is.cx = scr_w / 2;
    is.cy = scr_h / 2;

    is.win_changed = 0;

    write_cursor_pos(is.cx, is.cy);

    for (;;)
    {
        cmd_result_t cr;
        cr.win_changed = 0;

        input_frame_begin(&is);

        if (input_drain(mfd, &is))
        {
            write_cursor_pos(is.cx, is.cy);
        }
        is.win_changed = 0;

        cmd_process(&cr);

        bg_draw_full();
        render_all();
        cur_bake(is.cx, is.cy);
        comp_flush();

        for (volatile int i = 0; i < 500; i++)
            __asm__ volatile("pause");
    }
}
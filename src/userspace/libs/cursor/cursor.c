#include <emx/fb.h>
#include <emx/mouse.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include "cursor.h"

//config
#define T 0x00000000u
#define B 0xFF101010u
#define W 0xFFFFFFFFu
#define SCALE  1
#define INVERT 0

#define SCW (CW * SCALE)
#define SCH (CH * SCALE)

#define WM_DIRTY   "/tmp/wm/wm_dirty"
#define CURSOR_POS "/tmp/wm/cursor_pos"

static const unsigned int cursor_px[CW * CH] = {
    B,T,T,T,T,T,T,T,T,T,T,T,
    B,B,T,T,T,T,T,T,T,T,T,T,
    B,W,B,T,T,T,T,T,T,T,T,T,
    B,W,W,B,T,T,T,T,T,T,T,T,
    B,W,W,W,B,T,T,T,T,T,T,T,
    B,W,W,W,W,B,T,T,T,T,T,T,
    B,W,W,W,W,W,B,T,T,T,T,T,
    B,W,W,W,W,W,W,B,T,T,T,T,
    B,W,W,W,W,W,W,W,B,T,T,T,
    B,W,W,W,W,W,W,W,W,B,T,T,
    B,W,W,W,W,W,W,W,W,W,B,T,
    B,W,W,W,W,B,B,B,B,B,B,T,
    B,W,W,W,B,T,T,T,T,T,T,T,
    B,W,W,B,T,T,T,T,T,T,T,T,
    B,W,B,T,T,T,T,T,T,T,T,T,
    B,B,T,T,T,T,T,T,T,T,T,T,
};

static unsigned int scaled_cursor[SCW * SCH];
static unsigned int bg_save[SCW * SCH];
static int bg_valid = 0;
static int old_x = 0, old_y = 0;

static unsigned int invert_color(unsigned int c) {
    if (!INVERT) return c;
    if (c == T) return T;
    return (~c) | 0xFF000000u;
}

static void build_scaled_cursor(void) {
    for (int y = 0; y < CH; y++) {
        for (int x = 0; x < CW; x++) {
            unsigned int c = invert_color(cursor_px[y * CW + x]);
            for (int sy = 0; sy < SCALE; sy++) {
                for (int sx = 0; sx < SCALE; sx++) {
                    scaled_cursor[(y*SCALE+sy)*SCW + (x*SCALE+sx)] = c;
                }
            }
        }
    }
}

static int check_dirty(void) {
    int fd = open(WM_DIRTY, O_RDWR);
    if (fd < 0) return 0;
    char c = 0;
    read(fd, &c, 1);
    if (c == '1') {
        int dummy = 0;
        (void)dummy;
        close(fd);
        fd = open(WM_DIRTY, O_WRONLY | O_CREAT);
        if (fd >= 0) { write(fd, "0", 1); close(fd); }
        return 1;
    }
    close(fd);
    return 0;
}

int main(void)
{
    int fb = open("/dev/fb0", O_RDWR);
    int mfd = open("/dev/input/mouse0", O_RDONLY);
    if (fb < 0 || mfd < 0) _exit(1);

    build_scaled_cursor();

    for (;;) {
        mouse_event_t ev;
        if ((int)read(mfd, &ev, sizeof(ev)) < (int)sizeof(ev)) continue;

        if (check_dirty())
            bg_valid = 0;

        int nx = ev.abs_x;
        int ny = ev.abs_y;

        if (bg_valid) {
            fb_rect_t rst = {
                .x=(unsigned)old_x, .y=(unsigned)old_y,
                .w=SCW, .h=SCH, .pixels=bg_save
            };
            ioctl(fb, FBIO_BLIT, &rst);
        }

        fb_rect_t save = {
            .x=(unsigned)nx, .y=(unsigned)ny,
            .w=SCW, .h=SCH, .pixels=bg_save
        };
        ioctl(fb, FBIO_READ_RECT, &save);
        bg_valid = 1;
        old_x = nx;
        old_y = ny;

        fb_rect_t drw = {
            .x=(unsigned)nx, .y=(unsigned)ny,
            .w=SCW, .h=SCH, .pixels=scaled_cursor
        };
        ioctl(fb, FBIO_BLIT, &drw);

        {
            char pos[32];
            int i = 0;
            unsigned int vx = (unsigned)nx, vy = (unsigned)ny;
            char tx[12], ty[12];
            int li = 0;
            if (!vx) { tx[li++] = '0'; } else {
                char tmp[12]; int ti = 0;
                while (vx) { tmp[ti++] = '0' + vx % 10; vx /= 10; }
                while (ti > 0) tx[li++] = tmp[--ti];
            }
            tx[li] = '\0';
            li = 0;
            if (!vy) { ty[li++] = '0'; } else {
                char tmp[12]; int ti = 0;
                while (vy) { tmp[ti++] = '0' + vy % 10; vy /= 10; }
                while (ti > 0) ty[li++] = tmp[--ti];
            }
            ty[li] = '\0';
            int pi = 0;
            for (int k = 0; tx[k]; k++) pos[pi++] = tx[k];
            pos[pi++] = ' ';
            for (int k = 0; ty[k]; k++) pos[pi++] = ty[k];
            pos[pi++] = '\n'; pos[pi] = '\0';
            int pfd = open(CURSOR_POS, O_WRONLY | O_CREAT);
            if (pfd >= 0) { write(pfd, pos, (unsigned)pi); close(pfd); }
        }
    }
}
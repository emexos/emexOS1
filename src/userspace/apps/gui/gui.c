#include <emx/fb.h>
#include <emx/mouse.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#define T 0x00000000u
#define B 0xFF101010u // a bit lighter than black, but its lit unoticable
#define W 0xFFFFFFFFu

#define CW 9
#define CH 12

static const unsigned int cursor_px[CW * CH] = {
    B,T,T,T,T,T,T,T,T,
    B,B,T,T,T,T,T,T,T,
    B,W,B,T,T,T,T,T,T,
    B,W,W,B,T,T,T,T,T,
    B,W,W,W,B,T,T,T,T,
    B,W,W,W,W,B,T,T,T,
    B,W,W,W,W,W,B,T,T,
    B,W,W,W,W,W,W,B,T,
    B,W,W,B,B,B,B,B,T,
    B,W,B,T,T,T,T,T,T,
    B,B,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,
};

// saved background pixels so we can restore before each move
static unsigned int bg_save[CW * CH];
static int bg_valid = 0;
static int old_x = 0, old_y = 0;

int main(void) {
    int fb = open( "/dev/fb0", 0);
    int mfd = open("/dev/input/mouse0", 0);
    if (fb < 0 || mfd < 0) _exit(1);

    for (;;) {
        // wait for next mouse event from kernel ring buffer
        mouse_event_t ev;
        if ((int)read(mfd, &ev, sizeof(ev)) < (int)sizeof(ev)) continue;

        int nx = ev.abs_x;
        int ny = ev.abs_y;

        // restores the previous background before moving cursor
        if (bg_valid) {
            fb_rect_t rst = {
                .x = (unsigned int)old_x,
                .y = (unsigned int)old_y,
                .w = CW,
                .h = CH,
                .pixels = bg_save
            };
            ioctl(fb, FBIO_BLIT, &rst);
        }

        // save the new background at new position
        fb_rect_t save = {
            .x = (unsigned int)nx,
            .y = (unsigned int)ny,
            .w = CW,
            .h = CH,
            .pixels = bg_save
        };
        ioctl(fb, FBIO_READ_RECT, &save);
        bg_valid = 1;
        old_x = nx;
        old_y = ny;

        // draw the cursor at new position
        fb_rect_t drw = {
            .x = (unsigned int)nx,
            .y = (unsigned int)ny,
            .w = CW,
            .h = CH,
            .pixels = (unsigned int *)cursor_px
        };

        ioctl(fb, FBIO_BLIT, &drw);
    }
}
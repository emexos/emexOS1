#include "gen.h"

int fb;
int scr_w, scr_h, scr_pitch;
int win_x, win_y;

unsigned int row_buf[ROW_BUF_W];


#define CURSOR_POS "/tmp/cursor_pos"
#define CUR_W CW
#define CUR_H CH

// read cursor position from file written by cursor.c
static int read_cursor_pos(int *cx, int *cy) {
    int fd = open(CURSOR_POS, O_RDONLY);
    if (fd < 0) return 0;
    char buf[32];
    int n = (int)read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (n <= 0) return 0;
    buf[n] = '\0';
    int x = 0, y = 0, i = 0;
    while (buf[i] >= '0' && buf[i] <= '9') x = x * 10 + (buf[i++] - '0');
    while (buf[i] == ' ') i++;
    while (buf[i] >= '0' && buf[i] <= '9') y = y * 10 + (buf[i++] - '0');
    *cx = x; *cy = y;
    return 1;
}
static void erase_cursor_ghost(void) {
    int cx, cy;
    if (read_cursor_pos(&cx, &cy)) erase_rect(cx, cy, CUR_W, CUR_H);
}


// we never unlink it
// overwrite it
static void mark_dirty(void) {
    int fd = open(WM_DIRTY, O_WRONLY | O_CREAT);
    if (fd >= 0) { write(fd, "1", 1); close(fd); }
    erase_cursor_ghost(); // bad solution ik...
}
static void move_window(pid_t pid, int ox, int oy, int nx, int ny)
{
    if (ox == nx && oy == ny) return;

    int ix1 = nx > ox ? nx : ox;
    int iy1 = ny > oy ? ny : oy;
    int ix2 = (nx + WIN_W) < (ox + WIN_W) ? (nx + WIN_W) : (ox + WIN_W);
    int iy2 = (ny + WIN_H) < (oy + WIN_H) ? (ny + WIN_H) : (oy + WIN_H);
    int has_overlap = (ix1 < ix2 && iy1 < iy2);

    if (!has_overlap) {
        erase_rect(ox, oy, WIN_W, WIN_H);
    } else {
        if (oy < iy1) erase_rect(ox, oy,  WIN_W, iy1 - oy);
        if (oy + WIN_H > iy2) erase_rect(ox, iy2, WIN_W, (oy + WIN_H) - iy2);
        if (ox < ix1) erase_rect(ox, iy1, ix1 - ox, iy2 - iy1);
        if (ox + WIN_W > ix2) erase_rect(ix2,iy1, (ox + WIN_W) - ix2,iy2 - iy1);
    }

    win_x = nx;
    win_y = ny;
    draw_window(pid);
    mark_dirty();
}

int main(void)
{
    pid_t pid = fork();
    if (pid < 0) { printf("bg_test: fork failed\n"); return 1;}
    if (pid > 0) { printf("bg_test: pid=%d\n", (int)pid); return 0;}

    pid_t my_pid = getpid();

    fb = open("/dev/fb0", O_RDWR);
    if (fb < 0) _exit(1);

    int mouse = open("/dev/input/mouse0", O_RDONLY);

    graphics_init();

    int slot = claim_slot(my_pid);
    draw_background();

    int per_col = (scr_h - 20) / (WIN_H + WIN_GAP);
    if (per_col < 1) per_col = 1;

    if (slot == 0) {
        // first window goes in the center
        win_x = (scr_w - WIN_W) / 2;
        win_y = (scr_h - WIN_H) / 2;
    } else {
        win_x = (scr_w - WIN_W) / 2 - (slot / per_col) * (WIN_W + WIN_GAP + 4);
        win_y = (scr_h - WIN_H) / 2 + (slot % per_col) * (WIN_H + WIN_GAP);
        if (win_x < 0) win_x = 0;
        if (win_y + WIN_H > scr_h) win_y = scr_h - WIN_H;
    }

    draw_window(my_pid);
    mark_dirty();

    int dragging = 0;
    int drag_ox = 0;
    int	drag_oy = 0;
    int last_btn = 0;
    int idle_tick = 0;

    for (;;)
    {
        mouse_event_t ev;
        int got = (int)read(mouse, &ev, sizeof(ev));

        if (got == (int)sizeof(ev)) {
            int mx = ev.abs_x;
            int my = ev.abs_y;
            int btn = ev.buttons & MOUSE_BTN_LEFT;

            if (btn && !last_btn)
            {
                if (
                	mx >= win_x && mx < win_x + WIN_W &&
                    my >= win_y + 1 && my < win_y + TITLE_H + 1
                ){
                    dragging = 1;
                    drag_ox  = mx - win_x;
                    drag_oy  = my - win_y;
                }
            }

            if (!btn) dragging = 0;

            if (dragging) {
                int nx = mx - drag_ox;
                int ny = my - drag_oy;

                if (nx < 0) nx = 0;
                if (ny < 0) ny = 0;
                if (nx + WIN_W > scr_w) nx = scr_w - WIN_W;
                if (ny + WIN_H > scr_h) ny = scr_h - WIN_H;
                if (nx != win_x || ny != win_y) {
                    move_window(my_pid, win_x, win_y, nx, ny);
                    idle_tick = 0;
                }
            }

            last_btn = btn;
        }

        idle_tick++;

        if (idle_tick >= 400)
        {
            draw_window(my_pid);
            mark_dirty();
            idle_tick = 0;
        }

        for (volatile int i = 0; i < 20000; i++) __asm__ volatile("pause");
    }
}
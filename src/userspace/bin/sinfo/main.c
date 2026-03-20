#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <emx/mouse.h>
#include <emx/sinfo.h>

#include "wm.h"

#define GRAY_L 0xFFCCCCCCu
#define GRAY_D 0xFF333333u
#define CHECKER_SIZE 2

#define WIN_W 500
#define WIN_H 360
#define WIN_GAP 9
#define PAD 8

#define CURSOR_POS "/tmp/wm/cursor_pos"

draw_ctx g_ctx;
int win_x, win_y;

static unsigned int checker(int x, int y) {
    return (((x / CHECKER_SIZE) ^ (y / CHECKER_SIZE)) & 1) ? GRAY_D : GRAY_L;
}

static void draw_bg(void) {
    for (int y = 0; y < g_ctx.h; y++) {
        for (int x = 0; x < g_ctx.w && x < DS_ROW_BUF_W; x++)
            g_ctx.row_buf[x] = checker(x, y);
        ds_blit_row(&g_ctx, 0, y, g_ctx.w);
    }
}

static void erase_rect(int x, int y, int w, int h)
{
    for (int dy = 0; dy < h; dy++) {
        int sy = y + dy;
        if (sy < 0 || sy >= g_ctx.h) continue;
        for (int dx = 0; dx < w && dx < DS_ROW_BUF_W; dx++)
            g_ctx.row_buf[dx] = checker(x + dx, sy);
        ds_blit_row(&g_ctx, x, sy, w);
    }
}

static int read_cursor_pos(int *cx, int *cy)
{
	// opens the mouse device
    int fd = open(CURSOR_POS, O_RDONLY);
    int x = 0, y = 0, i = 0;
    char buf[32];
    int read2 = (int)read(fd, buf, sizeof(buf) - 1);

    if (fd < 0) return 0;
    close(fd);
    if (read2 <= 0) return 0;

    buf[read2] = '\0';

    while (buf[i] >= '0' && buf[i] <= '9') x = x * 10 + (buf[i++] - '0');
    while (buf[i] == ' ') i++;
    while (buf[i] >= '0' && buf[i] <= '9') y = y * 10 + (buf[i++] - '0');

    *cx = x; *cy = y;
    return 1;
}
/*
static void erase_cursor_ghost(void) {
    int cx, cy;
    if (read_cursor_pos(&cx, &cy)) erase_rect(cx, cy, CUR_W, CUR_H);
}
*/

// bad solution ik...
static void mark_dirty(void) {
	int cx, cy;
    int fd = open(WM_DIRTY, O_WRONLY | O_CREAT);

    if (fd >= 0) { write(fd, "1", 1); close(fd); }
    if (read_cursor_pos(&cx, &cy)) erase_rect(cx, cy, WM_FW, WM_FH);
}

static win_ui_config s_wcfg = { .title = "System Information", .style = WIN_STYLE_DEFAULT };

static common_ui_config make_win_cfg(void) {
    return (common_ui_config)
    {
        .point = { win_x, win_y },
        .size  = { WIN_W, WIN_H },
        .fg    = WM_BLACK,
        .flags = UI_FLAG_FOCUSED
    };
}

static void draw_window(pid_t pid)
{
	unsigned int v = (unsigned int)pid;
	int i = 0;
	int j = 0;

    static char pid_str[16];
    static char pid_line[32];
    const char *pfx = "pid: ";
    const char *lines[] =
    {
        "emexOS system information:",
        "kernel:  " __EMEXF_C__,
        "version: " __EMX_VER_,
        "build:   " __EMX_BUILD_,
        "arch:    x86-64",
        pid_line,
    };

    if (!v) {
    	pid_str[0] = '0'; pid_str[1] = '\0';
    }
    else {
        char tmp[12]; int ti = 0, si = 0;
        while (v) { tmp[ti++] = '0' + v % 10; v /= 10; }
        while (ti > 0) pid_str[si++] = tmp[--ti];
        pid_str[si] = '\0';
    }

    while (pfx[i]) pid_line[j++] = pfx[i++];

    i = 0;
    while (pid_str[i]) pid_line[j++] = pid_str[i++];
    pid_line[j] = '\0';

    // window text
    int n = (int)(sizeof(lines) / sizeof(lines[0]));

    common_ui_config content = window(&g_ctx, s_wcfg, make_win_cfg());

    for (int li = 0; li < n; li++)
    {
        label(&g_ctx, (text_ui_config) {
        	.text = lines[li]

        }, (common_ui_config)
    	{
            .point = { content.point.x + PAD, content.point.y + PAD + li * (WM_FH + 2) },
            .size  = { WIN_W - 2 * PAD, WM_FH },
            .fg    = WM_BLACK
        });
    }
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
        if (ox + WIN_W > ix2) erase_rect(ix2, iy1, (ox + WIN_W) - ix2, iy2 - iy1);
    }

    win_x = nx; win_y = ny;
    draw_window(pid);

    mark_dirty();
}

int main(void)
{
    pid_t pid = fork();
    if (pid < 0) {printf("bg_test: fork failed\n"); return 1; }
    if (pid > 0) {printf("bg_test: pid=%d\n", (int)pid); return 0; }

    pid_t my_pid = getpid();

    int fb_fd = open("/dev/fb0", O_RDWR);
    if (fb_fd < 0) _exit(1);
    int mouse = open("/dev/input/mouse0", O_RDONLY);

    ds_init(&g_ctx, fb_fd);

    int slot = claim_slot(my_pid);
    int per_col = (g_ctx.h - 20) / (WIN_H + WIN_GAP);
    if (per_col < 1) per_col = 1;

    draw_bg();

    if (slot == 0) {
        // first window goes in the center
        win_x = (g_ctx.w - WIN_W) / 2;
        win_y = (g_ctx.h - WIN_H) / 2;
    } else {
        win_x = (g_ctx.w - WIN_W) / 2 - (slot / per_col) * (WIN_W + WIN_GAP + 4);
        win_y = (g_ctx.h - WIN_H) / 2 + (slot % per_col) * (WIN_H + WIN_GAP);
        if (win_x < 0) win_x = 0;
        if (win_y + WIN_H > g_ctx.h) win_y = g_ctx.h - WIN_H;
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

        if (got == (int)sizeof(ev))
        {
            int mx = ev.abs_x;
            int my = ev.abs_y;
            int btn = ev.buttons & MOUSE_BTN_LEFT;

            if (btn && !last_btn) {
                if (wm_close_hit(make_win_cfg(), s_wcfg, mx, my)) {
                    erase_rect(win_x, win_y, WIN_W, WIN_H);
                    mark_dirty();
                    _exit(0);
                }

                if (
                    mx >= win_x && mx < win_x + WIN_W &&
                    my >= win_y + 1 && my < win_y + WM_TITLE_H + 1
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
                if (nx + WIN_W > g_ctx.w) nx = g_ctx.w - WIN_W;
                if (ny + WIN_H > g_ctx.h) ny = g_ctx.h - WIN_H;
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

        for (volatile int i = 0; i < 20000; i++)
        	__asm__ volatile("pause");
    }
}
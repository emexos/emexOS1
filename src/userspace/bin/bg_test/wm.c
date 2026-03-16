#include "gen.h"
#include "font_8x12.h"


static int slen(const char *s) {
    int n = 0;
    while (s[n]) n++;
    return n;
}
static void uint_to_str(unsigned int v, char *out) {
    if (!v) { out[0] = '0'; out[1] = '\0'; return; }
    char tmp[12];
    int i = 0;
    while (v) { tmp[i++] = '0' + v % 10; v /= 10; }
    int j = 0;
    while (i > 0) out[j++] = tmp[--i];
    out[j] = '\0';
}
static void buf_char(int bx, char c, unsigned int fg, unsigned int bg, int row) {
    unsigned int ci = (unsigned char)c & 0x7F;
    unsigned int bits = font_8x12[ci][row];
    for (int col = 0; col < FW; col++)
        row_buf[bx + col] = (bits & (1 << col)) ? fg : bg;
}
static void buf_str(int bx, const char *s, unsigned int fg, unsigned int bg, int row) {
    while (*s) {
        buf_char(bx, *s++, fg, bg, row);
        bx += FW;
    }
}
static unsigned int stripe(int x, int y) {
    (void)x;
    return (y & 1) ? BLACK : WHITE;
}

void draw_window(pid_t pid) {
    char pid_str[16];
    uint_to_str((unsigned int)pid, pid_str);

    const char *lines[] = {
    	"",
     	"emexOS system information:",
        "kernel:  " __EMEXF_C__,
        "version: " __EMX_VER_,
        "build:   " __EMX_BUILD_,
        "arch:    x86-64",
        "",
        "",
        "pid: ",
    };

    int title_text_w = slen(WM_TITLE) * FW;
    int title_text_x = (WIN_W - title_text_w) / 2;
    // stripes go from border
    int stripe_l_end = title_text_x - 4;
    int stripe_r_start = title_text_x + title_text_w + 4;

    int content_y = TITLE_H + TITLE_P;

    for (int dy = 0; dy < WIN_H; dy++) {
        for (int dx = 0; dx < WIN_W; dx++)
            row_buf[dx] = WHITE;

        // outer border
        if (dy < 2 || dy >= WIN_H - 2) {
            for (int dx = 0; dx < WIN_W; dx++)
                row_buf[dx] = BLACK;
        } else {
            row_buf[0] = BLACK; row_buf[1] = BLACK;
            row_buf[WIN_W - 1] = BLACK; row_buf[WIN_W - 2] = BLACK;
        }

        // title bar background
        if (dy >= 2 && dy < TITLE_H) {
            for (int dx = 2; dx < WIN_W - 2; dx++)
                row_buf[dx] = TITLE_BG;
        }

        // stripes on all sides of title
        if (dy >= 2 && dy < TITLE_H) {
            for (int dx = 2; dx < stripe_l_end; dx++)
                row_buf[dx] = stripe(dx, dy);
            for (int dx = stripe_r_start; dx < WIN_W - 2; dx++)
                row_buf[dx] = stripe(dx, dy);
        }

        // closed box (12x12)
        if (dy >= 3 && dy <= 3 + 12) {
        	//inset 4px from left border
            int bx = 6;
            // -2 cuz of border
            // == 4
            if (dy == 3 || dy == 3 + 12) {
                for (int dx = bx; dx <= bx + 12; dx++) row_buf[dx] = BLACK;
            } else {
                row_buf[bx] = BLACK; row_buf[bx + 12] = BLACK;
                for (int dx = bx + 1; dx < bx + 12; dx++) row_buf[dx] = WHITE;
            }
        }

        // title text is centered in title bar
        if (dy >= TITLE_P_B && dy < TITLE_P_B + FH && dy < TITLE_H) {
            buf_str(title_text_x, WM_TITLE, BLACK, WHITE, dy - TITLE_P_B);
        }

        // content lines
        for (int li = 0; li < 5; li++) {
            int rs = content_y + li * (FH + 2);
            if (dy >= rs && dy < rs + FH) {
                buf_str(PAD, lines[li], BLACK, WHITE, dy - rs);
                if (li == 4)
                    buf_str(PAD + 7 * FW, pid_str, BLACK, WHITE, dy - rs);
            }
        }

        // line under title bar
        if (dy == TITLE_H || dy == TITLE_H + TITLE_P_) {
            for (
            	int dx = TITLE_P; dx < WIN_W - TITLE_P; dx++
            ) row_buf[dx] = BLACK;
        }


        blit_row(win_x, win_y + dy, WIN_W, row_buf);
    }
}

static int sread(int fd, char *buf, int max) {
    int n = 0, r;
    buf[0] = '\0';

    while (n < max - 1) {
        r = (int)read(fd, buf + n, max - 1 - n);
        if (r <= 0) break;
        n += r;
    }
    buf[n] = '\0';
    return n;
}

static int count_lines(const char *buf) {
    int n = 0;
    for (int i = 0; buf[i]; i++)
        if (buf[i] == '\n') n++;
    return n;
}

int claim_slot(pid_t pid) {
    char buf[256];
    buf[0] = '\0';
    int slot = 0;

    int fd = open(WM_REG, O_RDONLY);
    if (fd >= 0) {
        sread(fd, buf, sizeof(buf));
        close(fd);
        slot = count_lines(buf);
    }

    fd = open(WM_REG, O_WRONLY | O_CREAT);
    if (fd >= 0) {
        char e[32];
        uint_to_str((unsigned int)pid, e);
        int el = slen(e);
        e[el] = '\n'; e[el + 1] = '\0';
        int bl = slen(buf);
        for (int i = 0; e[i]; i++) buf[bl + i] = e[i];
        buf[bl + el + 1] = '\0';
        write(fd, buf, (unsigned)slen(buf));
        close(fd);
    }
    return slot;
}
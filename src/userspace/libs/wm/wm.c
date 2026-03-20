#include "wm.h"
#include "../font8x12/font8x12.h"

#include <unistd.h>
#include <fcntl.h>


static int slen(const char *s) {
    int n = 0; while (s[n]) n++; return n;
}
static void scopy(char *dst, const char *src, int max) {
    int i = 0;
    while (i < max - 1 && src[i]) { dst[i] = src[i]; i++; }
    dst[i] = '\0';
}
static void uint_to_str(unsigned int v, char *out) {
    if (!v) { out[0] = '0'; out[1] = '\0'; return; }
    char tmp[12]; int i = 0;
    while (v) { tmp[i++] = '0' + v % 10; v /= 10; }
    int j = 0;
    while (i > 0) out[j++] = tmp[--i];
    out[j] = '\0';
}
static int str_to_uint(const char *s, unsigned int *out) {
    if (!*s) return 0;
    unsigned int v = 0;
    while (*s >= '0' && *s <= '9') { v = v * 10 + (unsigned)(*s++ - '0'); }
    *out = v;
    return 1;
}
static void buf_char(draw_ctx *ctx, int bx, char c, unsigned int fg, unsigned int bg, int row) {
    unsigned int bits = font8x12_glyph((unsigned char)c & 0x7F, row);
    for (int col = 0; col < WM_FW; col++)
        ctx->row_buf[bx + col] = (bits & (1u << col)) ? fg : bg;
}
static void buf_str(draw_ctx *ctx, int bx, const char *s, unsigned int fg, unsigned int bg, int row) {
    while (*s) { buf_char(ctx, bx, *s++, fg, bg, row); bx += WM_FW; }
}
static unsigned int stripe(int x, int y, int focused) {
    (void)x;
    if (focused) return (y & 1) ? WM_BLACK : WM_WHITE;
    return (y & 1) ? WM_SHADOW : WM_LIGHT;
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
    for (int i = 0; buf[i]; i++) if (buf[i] == '\n') n++;
    return n;
}

#define Z_BUF 2048
static void z_write(const char *buf) {

    int fd = open(WM_Z, O_WRONLY | O_CREAT);
    if (fd >= 0) { write(fd, buf, (unsigned)slen(buf)); close(fd); }
}
static const char *z_parse_line(const char *p, wm_entry_t *e)
{
    unsigned int pid = 0;
    str_to_uint(p, &pid);
    e->pid = (pid_t)pid;
    while (*p && *p != ':' && *p != '\n') p++;
    if (*p == ':') p++;
    int ti = 0;
    while (*p && *p != '\n' && ti < WM_TAG_MAX - 1) e->tag[ti++] = *p++;
    e->tag[ti] = '\0';
    if (*p == '\n') p++;
    return p;
}

void wm_register(pid_t pid, const char *tag) {

    static char buf[Z_BUF]; buf[0] = '\0';
    int fd = open(WM_Z, O_RDONLY);
    char entry[64];
    char pidstr[16];
    int ei = 0;
    int bl = slen(buf);
    uint_to_str((unsigned int)pid, pidstr);

    if (fd >= 0) { sread(fd, buf, sizeof(buf)); close(fd); }

    for (int k = 0; pidstr[k]; k++) entry[ei++] = pidstr[k];
    entry[ei++] = ':';

    for (int k = 0; tag[k] && k < WM_TAG_MAX - 1; k++) entry[ei++] = tag[k];

    entry[ei++] = '\n'; entry[ei] = '\0';

    for (int k = 0; entry[k]; k++) buf[bl + k] = entry[k];
    buf[bl + ei] = '\0';

    z_write(buf);
}
void wm_raise(pid_t pid)
{
    static char buf[Z_BUF]; buf[0] = '\0';
    int fd = open(WM_Z, O_RDONLY);
    if (fd >= 0) { sread(fd, buf, sizeof(buf)); close(fd); }

    // rebuild WITHOUT pid
    // try again
    static char out[Z_BUF]; out[0] = '\0';
    int oi = 0;
    char entry[64]; int ei = 0;

    // then fk it or whatever
    const char *p = buf;
    while (*p) {
        wm_entry_t e;
        p = z_parse_line(p, &e);
        char pidstr[16];
        uint_to_str((unsigned int)e.pid, pidstr);
        if (e.pid == pid) {
            // then save for appending later
            ei = 0;
            for (int k = 0; pidstr[k]; k++) entry[ei++] = pidstr[k];
            entry[ei++] = ':';
            for (int k = 0; e.tag[k]; k++) entry[ei++] = e.tag[k];
            entry[ei++] = '\n'; entry[ei] = '\0';
        } else {
            for (int k = 0; pidstr[k]; k++) out[oi++] = pidstr[k];
            out[oi++] = ':';
            for (int k = 0; e.tag[k]; k++) out[oi++] = e.tag[k];
            out[oi++] = '\n';
        }
    }

    // THEN append the raised entry at top
    for (int k = 0; entry[k]; k++) out[oi++] = entry[k];
    out[oi] = '\0';
    z_write(out);
}

void wm_unregister(pid_t pid)
{
    static char buf[Z_BUF]; buf[0] = '\0';
    int fd = open(WM_Z, O_RDONLY);
    int oi = 0;
    if (fd >= 0) { sread(fd, buf, sizeof(buf)); close(fd); }

    static char out[Z_BUF]; out[0] = '\0';
    const char *p = buf;

    while (*p) {
        wm_entry_t e;
        p = z_parse_line(p, &e);
        if (e.pid == pid) continue;
        char pidstr[16];
        uint_to_str((unsigned int)e.pid, pidstr);
        for (int k = 0; pidstr[k]; k++) out[oi++] = pidstr[k];
        out[oi++] = ':';
        for (int k = 0; e.tag[k]; k++) out[oi++] = e.tag[k];
        out[oi++] = '\n';
    }
    out[oi] = '\0';
    z_write(out);
}
int wm_z_list(wm_entry_t *out, int max)
{
    static char buf[Z_BUF]; buf[0] = '\0';
    int fd = open(WM_Z, O_RDONLY);
    if (fd >= 0) { sread(fd, buf, sizeof(buf)); close(fd); }

    int n = 0;
    const char *p = buf;
    while (*p && n < max) p = z_parse_line(p, &out[n++]);
    return n;
}
int wm_is_top(pid_t pid)
{
    static char buf[Z_BUF]; buf[0] = '\0';
    int fd = open(WM_Z, O_RDONLY);
    if (fd >= 0) { sread(fd, buf, sizeof(buf)); close(fd); }

    // walk to the last line
    wm_entry_t last; last.pid = 0;
    const char *p = buf;
    while (*p) p = z_parse_line(p, &last);
    return last.pid == pid;
}

int claim_slot(pid_t pid)
{
    static char buf[256]; buf[0] = '\0';
    int slot = 0;

    int fd = open(WM_REG, O_RDONLY);
    if (fd >= 0) { sread(fd, buf, sizeof(buf)); close(fd); slot = count_lines(buf); }

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

int wm_close_hit(common_ui_config cfg, win_ui_config wcfg, int mx, int my) {
    if (wcfg.style & WIN_STYLE_NOCLOSE) return 0;
    if (wcfg.style & WIN_STYLE_NOTITLE) return 0;
    int bx = cfg.point.x + 6;
    int by = cfg.point.y + 3;
    return mx >= bx && mx < bx + 12 && my >= by && my < by + 12;
}

common_ui_config window(draw_ctx *ctx, win_ui_config wcfg, common_ui_config cfg)
{
    if (cfg.flags & UI_FLAG_HIDDEN) { common_ui_config e = {0}; return e; }

    int wx = cfg.point.x, wy = cfg.point.y;
    int ww = cfg.size.w,  wh = cfg.size.h;
    int focused = (cfg.flags & UI_FLAG_FOCUSED) != 0;

    unsigned int bg         = cfg.bg    ? cfg.bg    : WM_WHITE;
    unsigned int col_border = WM_BLACK;
    unsigned int col_title  = bg;

    int tw = slen(wcfg.title) * WM_FW;
    int tx = (ww - tw) / 2;
    int sl = tx - 4;
    int sr = tx + tw + 4;

    for (int dy = 0; dy < wh; dy++) {
        for (int dx = 0; dx < ww && dx < DS_ROW_BUF_W; dx++) ctx->row_buf[dx] = bg;

        // 2px border
        if (dy < 2 || dy >= wh - 2) {
            for (int dx = 0; dx < ww; dx++) ctx->row_buf[dx] = col_border;
        } else {
            ctx->row_buf[0] = ctx->row_buf[1] = col_border;
            ctx->row_buf[ww-1] = ctx->row_buf[ww-2] = col_border;
        }

        if (!(wcfg.style & WIN_STYLE_NOTITLE)) {
            if (dy >= 2 && dy < WM_TITLE_H) {
                for (int dx = 2; dx < ww - 2; dx++) ctx->row_buf[dx] = col_title;
                for (int dx = 2;  dx < sl;    dx++) ctx->row_buf[dx] = stripe(dx, dy, focused);
                for (int dx = sr; dx < ww-2;  dx++) ctx->row_buf[dx] = stripe(dx, dy, focused);
            }

            if (dy >= WM_TITLE_P_B && dy < WM_TITLE_P_B + WM_FH && dy < WM_TITLE_H)
                buf_str(ctx, tx, wcfg.title, WM_BLACK, col_title, dy - WM_TITLE_P_B);

            if (dy == WM_TITLE_H || dy == WM_TITLE_H - 1)
                for (int dx = 0; dx < ww; dx++) ctx->row_buf[dx] = col_border;

            // close box
            if (!(wcfg.style & WIN_STYLE_NOCLOSE) && dy >= 3 && dy <= 3 + 12) {
                int bx = 6;
                if (dy == 3 || dy == 3 + 12) {
                    for (int dx = bx; dx <= bx + 12; dx++) ctx->row_buf[dx] = col_border;
                } else {
                    ctx->row_buf[bx] = ctx->row_buf[bx + 12] = col_border;
                    for (int dx = bx + 1; dx < bx + 12; dx++) ctx->row_buf[dx] = bg;
                }
            }
        }

        ds_blit_row(ctx, wx, wy + dy, ww);
    }

    int th  = (wcfg.style & WIN_STYLE_NOTITLE) ? 0 : WM_TITLE_H + 1;
    int pad = (cfg.flags & UI_FLAG_NO_PAD) ? 0 : 2;

    common_ui_config content = {
        .point    = { wx + 2 + pad, wy + th + 2 + pad },
        .size     = { ww - 4 - pad * 2, wh - th - 4 - pad * 2 },
        .fg       = cfg.fg,
        .bg       = bg,
        .border   = cfg.border,
        .title_fg = cfg.title_fg,
        .title_bg = cfg.title_bg,
        .flags    = cfg.flags
    };
    return content;
}

common_ui_config label(draw_ctx *ctx, text_ui_config tcfg, common_ui_config cfg)
{
    if (cfg.flags & UI_FLAG_HIDDEN) return cfg;

    int tx = cfg.point.x, ty = cfg.point.y;
    int tw = cfg.size.w > 0 ? cfg.size.w : slen(tcfg.text) * WM_FW;
    unsigned int bg = cfg.bg ? cfg.bg : WM_WHITE;

    for (int row = 0; row < WM_FH; row++) {
        for (int i = 0; i < tw && i < DS_ROW_BUF_W; i++) ctx->row_buf[i] = bg;
        buf_str(ctx, 0, tcfg.text, cfg.fg, bg, row);
        ds_blit_row(ctx, tx, ty + row, tw);
    }

    common_ui_config out = cfg;
    out.size.w = tw; out.size.h = WM_FH;
    return out;
}

common_ui_config textbox(draw_ctx *ctx, text_ui_config tcfg, common_ui_config cfg)
{
    if (cfg.flags & UI_FLAG_HIDDEN) return cfg;

    int tx = cfg.point.x, ty = cfg.point.y;
    int tw = cfg.size.w > 0 ? cfg.size.w : slen(tcfg.text) * WM_FW + 4;
    int th = cfg.size.h > 0 ? cfg.size.h : WM_FH + 4;
    unsigned int col_border = cfg.border ? cfg.border : WM_BLACK;
    unsigned int inner_bg = cfg.bg ? cfg.bg: WM_WHITE;

    ds_rect(ctx, tx, ty, tw, th, inner_bg);
    ds_rect_outline(ctx, tx, ty, tw, th, col_border);

    int iw = tw - 4;
    for (int row = 0; row < WM_FH && row < th - 4; row++) {
        for (int i = 0; i < iw && i < DS_ROW_BUF_W; i++) ctx->row_buf[i] = inner_bg;
        buf_str(ctx, 0, tcfg.text, cfg.fg, inner_bg, row);
        ds_blit_row(ctx, tx + 2, ty + 2 + row, iw);
    }

    return cfg;
}
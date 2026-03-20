#include <unistd.h>
#include <fcntl.h>
//#include <string.h>
//#include <stdio.h>

#include "wm.h"
#include <stdio.h>

#define LOGIN_W 300
#define LOGIN_H 164
#define LOGIN_PAD 10

// TODO:
// - read from /emr/config/user.emcg
// - implement a emex config file parser lib
//
#define LOGIN_USER "emex"
#define LOGIN_PASS "emex"
#define USERNAME "username: "
#define PASSWORD "password: "
#define MAX_TRIES 3
#define BUF_SIZE 64

#define KBD_PATH "/dev/input/keyboard0"

typedef struct {
    unsigned int keycode;
    unsigned int modifiers;
    unsigned char pressed;
} key_event_t;

static draw_ctx ctx;

static win_ui_config s_wcfg = {
    .title = "login",
    .style = WIN_STYLE_NOCLOSE,
    //.title_bg     = 0xFF007700u,
    //.title_fg     = 0xFFFFFFFFu,
    //.title_shadow = 0xFF000000u,
    //.title_stripe = 0xFF000000u,
};

static int str_eq(const char *a, const char *b) {
    while (*a && *b) { if (*a != *b) return 0; a++; b++; }
    return *a == '\0' && *b == '\0';
}

static void draw_bg(void) {
    for (int y = 0; y < ctx.h; y++) {
        for (int x = 0; x < ctx.w && x < DS_ROW_BUF_W; x++)
            ctx.row_buf[x] = WM_FACE;
        ds_blit_row(&ctx, 0, y, ctx.w);
    }
}

static void mark_dirty(void) {
    int fd = open(WM_DIRTY, O_WRONLY | O_CREAT);
    if (fd >= 0) { write(fd, "1", 1); close(fd); }
}

static void draw_login (
    int wx, int wy,
    const char *user_buf, int pass_len,
    const char *msg, int field
){
    static char pass_mask[BUF_SIZE + 1];
    int i;
    for (i = 0; i < pass_len && i < BUF_SIZE; i++) pass_mask[i] = '*';
    pass_mask[i] = '\0';

    common_ui_config content = window(&ctx, s_wcfg,
    (common_ui_config) {
        .point = { wx, wy },
        .size  = { LOGIN_W, LOGIN_H },
        .fg    = WM_BLACK,
        .flags = UI_FLAG_NO_MOVE | UI_FLAG_FOCUSED
    });

    int cx = content.point.x + LOGIN_PAD;
    int cy = content.point.y + LOGIN_PAD;

    label(&ctx,
        (text_ui_config){ .text = USERNAME },
        (common_ui_config)
    {
        .point = { cx, cy },
        .size  = { LOGIN_W - LOGIN_PAD * 2, WM_FH },
        .fg    = WM_BLACK
    });

    cy += WM_FH + 2;

    textbox(&ctx,
        (text_ui_config){ .text = user_buf },
        (common_ui_config)
    {
        .point  = { cx, cy },
        .size   = { LOGIN_W - LOGIN_PAD * 2, WM_FH + 6 },
        .fg     = WM_BLACK,
        .border = (field == 0) ? 0xFF000080u : WM_BLACK
    });

    cy += WM_FH + 6 + 6;

    label(&ctx,
        (text_ui_config){ .text = PASSWORD },
        (common_ui_config)
    {
        .point = { cx, cy },
        .size  = { LOGIN_W - LOGIN_PAD * 2, WM_FH },
        .fg    = WM_BLACK
    });

    cy += WM_FH + 2;

    textbox(&ctx,
        (text_ui_config){ .text = pass_mask },
        (common_ui_config)
    {
        .point  = { cx, cy },
        .size   = { LOGIN_W - LOGIN_PAD * 2, WM_FH + 6 },
        .fg     = WM_BLACK,
        .border = (field == 1) ? 0xFF000080u : WM_BLACK
    });

    cy += WM_FH + 6 + 6;

    if (msg && *msg) {
        label(&ctx,
            (text_ui_config){ .text = msg },
            (common_ui_config)
    	{
            .point = { cx, cy },
            .size  = { LOGIN_W - LOGIN_PAD * 2, WM_FH },
            .fg    = 0xFFCC0000u
        });
    }
    //printf("\033[0m----------------------\n");
    //printf("\033[0m\n too many failed attempts\n");
}

int main(void)
{
	printf("debug: login started \n");
    int fb_fd = open("/dev/fb0", O_RDWR);
    int kbd = open(KBD_PATH,  O_RDONLY);
    if (fb_fd < 0 || kbd < 0) return 1;

    printf("debug: init display services\n");

    ds_init(&ctx, fb_fd);

    int wx = (ctx.w - LOGIN_W) / 2;
    int wy = (ctx.h - LOGIN_H) / 2;

    draw_bg();

    int field = 0;
    int attempts = 0;

    char user_buf[BUF_SIZE]; user_buf[0] = '\0'; int user_len = 0;
    char pass_buf[BUF_SIZE]; pass_buf[0] = '\0'; int pass_len = 0;
    char msg[64]; msg[0]= '\0';

    draw_login(wx, wy, user_buf, pass_len, msg, field);
    mark_dirty();

    for (;;)
    {
        key_event_t ev;
        int n = (int)read(kbd, &ev, sizeof(ev));
        if (n != (int)sizeof(ev)) continue;
        if (!ev.pressed) continue;

        unsigned int kc = ev.keycode;
        char c = (char)(kc & 0xFF);

        if (kc == '\t') {
            field = field ? 0 : 1;
        } else if (kc == '\n' || kc == '\r') {
            if (field == 0) {
                field = 1;
            } else
            {
                if (str_eq(user_buf, LOGIN_USER) && str_eq(pass_buf, LOGIN_PASS)) return 0;

                attempts++;
                pass_len = 0; pass_buf[0] = '\0';
                field = 0;
                if (attempts >= MAX_TRIES) return 2;

                const char *pfx = "wrong ("; const char *sfx = " left)";
                int mi = 0;
                for (int k = 0; pfx[k]; k++) msg[mi++] = pfx[k];
                msg[mi++] = '0' + (MAX_TRIES - attempts);
                for (int k = 0; sfx[k]; k++) msg[mi++] = sfx[k];
                msg[mi] = '\0';
            }
        } else if (kc == '\b') {
            if (field == 0 && user_len > 0) user_buf[--user_len] = '\0';
            else if (field == 1 && pass_len > 0) pass_buf[--pass_len] = '\0';
        } else if (c >= 0x20 && c <= 0x7E) {
            if (field == 0 && user_len < BUF_SIZE - 1) { user_buf[user_len++] = c; user_buf[user_len] = '\0'; }
            else if (field == 1 && pass_len < BUF_SIZE - 1) { pass_buf[pass_len++] = c; pass_buf[pass_len] = '\0'; }
        }

        draw_login(wx, wy, user_buf, pass_len, msg, field);
        mark_dirty();
    }
}

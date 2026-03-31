#include <unistd.h>
#include <fcntl.h>
#include <string.h>
//#include <stdio.h>

#include "libdesktop.h"
#include "emxfb0.h"
#include "font8x12.h"
#include "login.h"
#include <stdio.h>

//-////////////////////////////////////////-//
//-//                                    //-//
//-//            emex login              //-//
//-//          graphical login           //-//
//-//                                    //-//
//-////////////////////////////////////////-//

#define LOGIN_W 300
#define LOGIN_H 164
#define LOGIN_PAD 10

#define APPTITLE "login"

// TODO:
// - read from /emr/config/user.emcg
// - implement a emex config file parser lib
//
//#define LOGIN_USER "emex"
//#define LOGIN_PASS "emex"
#define USERNAME "username: "
#define PASSWORD "password: "
//#define MAX_TRIES 3
//#define BUF_SIZE 64
char log_user[MAX_TRIES][BUF_SIZE];
char log_pass[MAX_TRIES][BUF_SIZE];

#define KBD_PATH "/dev/input/keyboard0"

//                   AARRGGBB
#define LG_BG      0xFFD4D0C8u
#define LG_BLACK   0xFF000000u
#define LG_WHITE   0xFFFFFFFFu
#define LG_FOCUSED 0xFF000080u
#define LG_ERR     0xFFCC0000u
#define LG_FG      0xFF000000u

#define FW FONT8X12_W
#define FH FONT8X12_H

typedef struct {
    unsigned int keycode;
    unsigned int modifiers;
    unsigned char pressed;
} key_event_t;

static draw_ctx g_ctx;
static int g_fb = -1;
static int g_kbd = -1;
static DesktopArea ca;

/*
static win_ui_config s_wcfg = {
    .title = "login",
    .style = WIN_STYLE_NOCLOSE,
    //.title_bg     = 0xFF007700u,
    //.title_fg     = 0xFFFFFFFFu,
    //.title_shadow = 0xFF000000u,
    //.title_stripe = 0xFF000000u,
};*/

static void fill_rect(int x, int y, int w, int h, unsigned int c)
{
    for (int dy = 0; dy < h; dy++)
    {
        for (int dx = 0; dx < w && dx < DS_ROW_BUF_W; dx++) g_ctx.row_buf[dx] = c;

        ds_blit_row(&g_ctx, x, y + dy, w);
    }
}

static void outline_rect(int x, int y, int w, int h, unsigned int c)
{
    for (int dx = 0; dx < w && dx < DS_ROW_BUF_W; dx++)
        g_ctx.row_buf[dx] = c;
    ds_blit_row(&g_ctx, x, y, w);
    ds_blit_row(&g_ctx, x, y + h - 1, w);
    for (int dy = 1; dy < h - 1; dy++)
    {
        g_ctx.row_buf[0] = c;
        ds_blit_row(&g_ctx, x, y + dy, 1);
        ds_blit_row(&g_ctx, x + w - 1, y + dy, 1);
    }
}

static void draw_char(int px, int py, char c, unsigned int fg, unsigned int bg)
{
    unsigned char uc = (unsigned char)c & 0x7Fu;
    for (int row = 0; row < FH; row++)
    {
        unsigned int bits = font8x12_glyph(uc, row);
        for (int col = 0; col < FW && col < DS_ROW_BUF_W; col++)
            g_ctx.row_buf[col] = (bits & (1u << col)) ? fg : bg;
        ds_blit_row(&g_ctx, px, py + row, FW);
    }
}

static void draw_str(int px, int py, const char *s, unsigned int fg, unsigned int bg)
{
    while (*s) { draw_char(px, py, *s++, fg, bg); px += FW; }
}

static void draw_field(
	int x, int y, int w, int h,
    const char *text, int focused
) {
    fill_rect(x, y, w, h, LG_WHITE);
    outline_rect(x, y, w, h, focused ? LG_FOCUSED : LG_BLACK);
    if (text && text[0])
        draw_str(x + 3, y + (h - FH) / 2, text, LG_FG, LG_WHITE);
}


static void draw_login(
	const char *user_buf, int pass_len, const char *msg, int field)
{
    int cx = ca.x + LOGIN_PAD;
    int cy = ca.y + LOGIN_PAD;

    fill_rect(ca.x, ca.y, ca.w, ca.h, LG_BG);

    // username
    draw_str(cx, cy, "username:", LG_FG, LG_BG);
    cy += FH + 3;
    draw_field(cx, cy, ca.w - LOGIN_PAD * 2, FH + 6, user_buf, field == 0);
    cy += FH + 6 + 6;

    // password
    draw_str(cx, cy, "password:", LG_FG, LG_BG);
    cy += FH + 3;

    char mask[BUF_SIZE + 1];
    int i;
    for (i = 0; i < pass_len && i < BUF_SIZE; i++) mask[i] = '*';
    mask[i] = '\0';
    draw_field(cx, cy, ca.w - LOGIN_PAD * 2, FH + 6, mask, field == 1);
    cy += FH + 6 + 6;

    if (msg && msg[0]) draw_str(cx, cy, msg, LG_ERR, LG_BG);

    desktop.markDirty();
}

static int str_eq(const char *a, const char *b) {
    while (*a && *b) { if (*a != *b) return 0; a++; b++; }
    return *a == '\0' && *b == '\0';
}

int main(void)
{
	printf("debug: login started \n");
    g_fb = open("/dev/fb0", O_RDWR);
    g_kbd = open(KBD_PATH, O_RDONLY);
    if (g_fb < 0 || g_kbd < 0) return 1;

    printf("debug: init display services\n");
    ds_init(&g_ctx, g_fb);

    int win_x = (g_ctx.w - LOGIN_W) / 2;
    int win_y = (g_ctx.h - LOGIN_H) / 2;
    if (win_x < 0) win_x = 0;
    if (win_y < 0) win_y = 0;

    // DT_NOMOVE means that the window cant be moved                       DT_NOMOVE
    desktop.createWindow(APPTITLE, win_x, win_y, LOGIN_W, LOGIN_H,DT_WIN  | DT_NOMOVE);
    ca = desktopContentArea(win_x, win_y, LOGIN_W, LOGIN_H, DT_WIN       | DT_NOMOVE);

    int field = 0;
    int attempts = 0;

    char user_buf[BUF_SIZE]; user_buf[0] = '\0'; int user_len = 0;
    char pass_buf[BUF_SIZE]; pass_buf[0] = '\0'; int pass_len = 0;
    char msg[64]; msg[0]= '\0';

    // draw immediately so login appears without requiring a keypress
    draw_login(user_buf, pass_len, msg, field);

    for (;;) {
        key_event_t ev;
        int n = (int)read(g_kbd, &ev, sizeof(ev));
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
                if (str_eq(user_buf, LOGIN_USER) && str_eq(pass_buf, LOGIN_PASS)) {
                    desktop.closeWindow();
                    return 0;
                }
                // save attempts
                if (attempts < MAX_TRIES)
                {
                    strncpy(log_user[attempts], user_buf, BUF_SIZE);
                    strncpy(log_pass[attempts], pass_buf, BUF_SIZE);
                }

                attempts++;

                pass_len = 0;
                pass_buf[0] = '\0';
                field = 0;

                if (attempts >= MAX_TRIES)
                {
                	/*
	                 * writes to login.log
	                 */
					int fd = open(LOG_PATH, O_WRONLY);
					if (fd < 0) fd = open(LOG_PATH, O_WRONLY | O_CREAT);
					if (fd >= 0)
					{
						write(fd, "[LOGIN] login attempt failed;\n", 30);

						for (int i = 0; i < MAX_TRIES; i++)
						{
					    	char buf[256];
						    int len = snprintf(buf, sizeof(buf),
					        "[LOGIN] %d:\n[LOGIN] u: %s\n[LOGIN] p: %s\n",
					        i + 1,
					        log_user[i],
					        log_pass[i]
					    );
					    write(fd, buf, len);
					}

					close(fd);
					}
                    desktop.closeWindow();
                    return 2;
                }

                const char *pfx = "wrong ("; const char *sfx = " left)";
                int mi = 0;
                for (int k = 0; pfx[k]; k++) msg[mi++] = pfx[k];
                msg[mi++] = '0' + (MAX_TRIES - attempts);

                for (int k = 0; sfx[k]; k++) msg[mi++] = sfx[k];
                msg[mi] = '\0';
            }
        } else if (kc == '\b')
        {
            if (field == 0 && user_len > 0) user_buf[--user_len] = '\0';
            else if (field == 1 && pass_len > 0) pass_buf[--pass_len] = '\0';
        } else if (c >= 0x20 && c <= 0x7E)
        {
            if (field == 0 && user_len < BUF_SIZE - 1) {
                user_buf[user_len++] = c;
                user_buf[user_len]   = '\0';
            } else if (field == 1 && pass_len < BUF_SIZE - 1) {
                pass_buf[pass_len++] = c;
                pass_buf[pass_len]   = '\0';
            }
        }

        draw_login(user_buf, pass_len, msg, field);
    }

}

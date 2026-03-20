#pragma once

#include <sys/types.h>
#include "../emxfb0/emxfb0.h"

#define WM_REG   "/tmp/wm/wm_reg"
#define WM_DIRTY "/tmp/wm/wm_dirty"
#define WM_Z     "/tmp/wm/wm_z" // btw z order is shit :(

// cuz it doesnt work :/


// colors
// some aren't used anymore cuz of stuf i removed
#define WM_BLACK     0xFF000000u
#define WM_WHITE     0xFFFFFFFFu
#define WM_FACE      0xFFD4D0C8u
#define WM_HILIGHT   0xFFFFFFFFu
#define WM_LIGHT     0xFFDFDFDFu
#define WM_SHADOW    0xFF808080u
#define WM_DKSHADOW  0xFF404040u
#define WM_TITLE_ACT 0xFF000080u
#define WM_TITLE_INA 0xFF808080u // inner
#define WM_TITLE_TXT 0xFFFFFFFFu // title text

#define WM_TITLE_H  18
#define WM_TITLE_P_B 2
#define WM_BTN_SZ   14

#define WM_FW 8
#define WM_FH 12

typedef struct { int x, y; } ui_pt_t;
typedef struct { int w, h; } ui_sz_t;

// color fields
// 0 == use default
//
typedef struct {
    ui_pt_t point;
    ui_sz_t size;
    unsigned int fg;
    unsigned int bg;
    unsigned int border;
    unsigned int title_fg;
    unsigned int title_bg;
    unsigned int flags;
} common_ui_config;


#define UI_FLAG_NO_MOVE (1u << 0)
#define UI_FLAG_HIDDEN  (1u << 1)
#define UI_FLAG_FOCUSED (1u << 2)
#define UI_FLAG_NO_PAD  (1u << 3)

typedef struct {
	const char *text;
} text_ui_config;
typedef struct {
    const char  *title;
    unsigned int style;
} win_ui_config;

#define WIN_STYLE_DEFAULT 0
#define WIN_STYLE_NOCLOSE (1u << 0)
#define WIN_STYLE_NOTITLE (1u << 1)


#define DRAW(item, children)        \
({                                  \
    common_ui_config parent = item; \
    (void)parent;                   \
    children;                       \
})

#define RELATIVE(a, b) { parent.point.x + (a), parent.point.y + (b) }


// one entry in the z-order registry
#define WM_TAG_MAX 32
typedef struct {
    pid_t pid;
    char  tag[WM_TAG_MAX];
} wm_entry_t;


// i prob should go wayland-like right?
//
void wm_register(pid_t pid, const char *tag);
void wm_raise(pid_t pid);
void wm_unregister(pid_t pid);
int wm_z_list(wm_entry_t *out, int max);
int wm_is_top(pid_t pid);

int wm_close_hit(common_ui_config cfg, win_ui_config wcfg, int mx, int my);
int claim_slot(pid_t pid);

// smth like different widgets in windows/apps/whatever
common_ui_config window(draw_ctx *ctx, win_ui_config  wcfg, common_ui_config cfg);
common_ui_config label(draw_ctx *ctx, text_ui_config tcfg, common_ui_config cfg);
common_ui_config textbox(draw_ctx *ctx, text_ui_config tcfg, common_ui_config cfg);
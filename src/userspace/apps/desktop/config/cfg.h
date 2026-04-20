#pragma once

#define DT_DIR    "/tmp/dt"
#define DT_CMD    "/tmp/dt/cmd"
#define DT_DIRTY  "/tmp/dt/dirty"
#define DT_CURSOR "/tmp/dt/cursor"
#define MOUSE_DEV "/dev/input/mouse0"

#define DT_BG 0xFF008080u  // win95/98 teal ig

// window frame
#define DT_BLACK     0xFF000000u
#define DT_WHITE     0xFFFFFFFFu
#define DT_FACE      0xFFD4D0C8u
#define DT_LIGHT     0xFFDFDFDFu
#define DT_SHADOW    0xFF808080u

// window title bar
#define DT_TITLE_ACT 0xFF000080u  // when focused
#define DT_TITLE_INA 0xFF808080u  // when unfocused
#define DT_TITLE_TXT 0xFFFFFFFFu  // text of titlebar

// titlebar config
#define DT_TITLE_H  18 // tot. inc. high
#define DT_TITLE_PB 2  // pad. top
#define DT_BORDER   1  // border width

#define DT_CLOSE_X  7  // x offset
#define DT_CLOSE_Y  5  // y offset
#define DT_CLOSE_SZ 8 // square size

// font rendering
#define DT_FW 8
#define DT_FH 12
#include "stdclrs.h"
// ******************
// kernel theme:
//

//bootup
#define BOOTUP_VISUALS 0 // verbose boot == 0, silent boot == 1
// macros
#if BOOTUP_VISUALS == 0
    #define BOOTUP_PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__)
    #define BOOTUP_PRINT(msg, col) print(msg, col)
    #define BOOTUP_PRINT_INT(num, col) printInt(num, col)
#else
    #define BOOTUP_PRINTF(fmt, ...) ((void)0)
    #define BOOTUP_PRINT(msg, col) ((void)0)
    #define BOOTUP_PRINT_INT(num, col) ((void)0)
#endif
//screen:
#define BOOTSCREEN_BG_COLOR GFX_ST_BLACK
#define BOOTSCREEN_COLOR GFX_ST_WHITE
#define LOGO_SCALE 1
#define BOOTUP_COLOR_THEME STD

//console
//#define CONSOLESCREEN_COLOR 0xFF3B4252
#define CONSOLESCREEN_BG_COLOR GFX_BG
#define CONSOLESCREEN_COLOR GFX_WHITE
#define CONSOLE_COLOR_THEME FLU

//panic
//
#define PANICSCREEN_BG_COLOR GFX_ST_BLUE
#define PANICSCREEN_COLOR GFX_ST_WHITE
#define PANICSCREEN_COLOR_R GFX_ST_RED
#define PANIC_COLOR_THEME STD


/*
// ******************
// userspace theme:
//

//desktop
#define BACKGROUND_COLOR 0xFF3B4252

#define IMOUSE_COLOR 0xFFFFFFFF // inner color
#define OMOUSE_COLOR 0xFFFFFFFF // outer color
//TODO: implement mouse when usb driver

*/

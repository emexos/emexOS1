#include "stdclrs.h"

#include <kernel/file_systems/vfs/vfs.h> // fs_write & fs_open
#include <klib/debug/serial.h>

// ******************
// kernel theme:
//

//bootup
extern int init_boot_log;
#define BOOTUP_VISUALS 0 // verbose boot == 0, silent boot == 1
// macros
#if BOOTUP_VISUALS == 0
    #define BOOTUP_PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__)
    #define BOOTUP_PRINT(msg, col) \
        do { \
            print(msg, col); \
            if (init_boot_log >= 0) fs_write(init_boot_log, msg, str_len(msg)); \
        } while(0)

    #define BOOTUP_PRINT_INT(num, col) \
        do { \
            printInt(num, col); \
            if (init_boot_log >= 0) { \
                char buf[12]; \
                buf[0] = '\0'; \
                str_append_uint(buf, (u32)num); \
                fs_write(init_boot_log, buf, str_len(buf)); \
            } \
        } while(0)

#else
    #define BOOTUP_PRINTF(fmt, ...) ((void)0)
    #define BOOTUP_PRINT(msg, col) \
        do { \
            printf("%s", msg); \
            if (init_boot_log >= 0) fs_write(init_boot_log, msg, str_len(msg)); \
        } while(0)

    #define BOOTUP_PRINT_INT(num, col) \
        do { \
            printInt(num, col); \
            if (init_boot_log >= 0) { \
                char buf[12]; \
                buf[0] = '\0';
                str_append_uint(buf, num); \
                fs_write(init_boot_log, buf, str_len(buf)); \
            } \
        } while(0)
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

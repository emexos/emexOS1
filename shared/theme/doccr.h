#include "stdclrs.h"

#include <kernel/file_systems/vfs/vfs.h> // fs_write & fs_open
#include <kernel/communication/serial.h>
#include <string/string.h>
#include <config/system.h>

#define PCF_FONT_PATH "/boot/fonts/gohu-uni-14.pcf"

// ******************
// kernel theme:
//

//bootup
extern int init_boot_log;
#define BOOTUP_FONT FONT_8X8_DOS

// macros
#if BOOTUP_VISUALS == 0
    // Verbose boot - zeigt alles am Bildschirm UND loggt
    #define BOOTUP_PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__)
    //--------------------------------------
    #define BOOTUP_PRINT(msg, col) \
        do { \
            print(msg, col); \
            if (init_boot_log >= 0) { \
                fs_write(init_boot_log, msg, str_len(msg)); \
            } \
        } while(0)
    //--------------------------------------
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
    //--------------------------------------
    #define BOOTUP_PRINTBS(msg, col) \
        do { \
            printbs(msg, col); \
            if (init_boot_log >= 0) { \
                fs_write(init_boot_log, msg, str_len(msg)); \
            } \
        } while(0)
    //--------------------------------------

#else
    // Silent boot - nur logging, keine Bildschirmausgabe
    #define BOOTUP_PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__)
    //--------------------------------------
    #define BOOTUP_PRINT(msg, col) \
        do { \
            if (init_boot_log >= 0) { \
                fs_write(init_boot_log, msg, str_len(msg)); \
            } \
        } while(0)
    //--------------------------------------
    #define BOOTUP_PRINT_INT(num, col) \
        do { \
            if (init_boot_log >= 0) { \
                char buf[12]; \
                buf[0] = '\0'; \
                str_append_uint(buf, (u32)num); \
                fs_write(init_boot_log, buf, str_len(buf)); \
            } \
        } while(0)
    //--------------------------------------
    #define BOOTUP_PRINTBS(msg, col) \
        do { \
            if (init_boot_log >= 0) { \
                fs_write(init_boot_log, msg, str_len(msg)); \
            } \
        } while(0)
    //--------------------------------------

#endif

//screen:
#define BOOTSCREEN_BG_COLOR GFX_ST_BLACK
#define BOOTSCREEN_COLOR GFX_ST_WHITE
#define LOGO_SCALE 2
#define BOOTUP_COLOR_THEME STD

//console
#define CONSOLE_FONT FONT_8X8_DOS

#define CONSOLESCREEN_BG_TYPE 0  // 0 = color, 1 = bmp picture
#define CONSOLESCREEN_BG_COLOR GFX_BG
#define CONSOLESCREEN_BG_IMAGE "/images/bg.bmp"

#define CONSOLESCREEN_COLOR GFX_WHITE
#define CONSOLE_COLOR_THEME FLU

//panic
#define PANIC_FONT FONT_8X8_DOS
#define PANICSCREEN_BG_COLOR GFX_ST_BLUE
#define PANICSCREEN_COLOR GFX_ST_WHITE
#define PANICSCREEN_COLOR_R GFX_ST_RED
#define PANIC_COLOR_THEME STD

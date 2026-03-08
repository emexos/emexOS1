#ifndef THEME_H
#define THEME_H

#include <types.h>

#ifdef __cplusplus
extern "C" {
#endif

// theme types
typedef enum {
    THEME_STD = 0,
    THEME_FLU = 1
} ThemeType;
//context types
typedef enum {
    THEME_BOOTUP = 0,
    THEME_CONSOLE = 1,
    THEME_PANIC = 2
} ThemeContext;

// unified naming
typedef enum {
    COLOR_BLACK = 0,
    COLOR_BG,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_YELLOW,
    COLOR_BLUE,
    COLOR_PURPLE,
    COLOR_CYAN,
    COLOR_WHITE
} ThemeColor;
typedef struct {
    u32 BLACK;
    u32 BG;
    u32 RED;
    u32 GREEN;
    u32 YELLOW;
    u32 BLUE;
    u32 PURPLE;
    u32 CYAN;
    u32 WHITE;
} ThemeColors;


void theme_init();
void setcontext(ThemeContext context);
ThemeContext getcontext();

void sbootup_theme(ThemeType type);
//void sconsole_theme(ThemeType type);
void spanic_theme(ThemeType type);
void reload_console_theme(void);

u32 get_color(ThemeColor color);


// current colors from theme
u32 black();
u32 bg();
u32 red();
u32 green();
u32 yellow();
u32 blue();
u32 purple();
u32 cyan();
u32 white();

#ifdef __cplusplus
}
#endif

#endif

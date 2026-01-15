#include "theme.h"


// if fs is implemented these colors should be loaded from a stdclrs.toml file or any kind of format
static const ThemeColors STD_THEME = {
    0xFF000000, /* BLACK */
    0xFF000000, /* BG */
    0xFFFF0000, /* RED */
    0xFF00FF00, /* GREEN */
    0xFFFFFF00, /* YELLOW */
    0xFF0000FF, /* BLUE */
    0xFF800080, /* PURPLE */
    0xFF00FFFF, /* CYAN */
    0xFFFFFFFF  /* WHITE */
};

static const ThemeColors FLU_THEME = {
    0xFF111111, /* BLACK */
    0xFF1F1F1F, /* BG */
    0xFF9E6E6E, /* RED */
    0xFF7A8A7A, /* GREEN */
    0xFFB8A788, /* YELLOW */
    0xFF6E7F8E, /* BLUE */
    0xFF857A8E, /* PURPLE */
    0xFF7A8E8E, /* CYAN */
    0xFFD8D8D8  /* WHITE */
};


// for different contexts
static const ThemeColors* bootup_theme = &STD_THEME;
static const ThemeColors* console_theme = &FLU_THEME;
static const ThemeColors* panic_theme = &STD_THEME;
static ThemeContext current_context = THEME_BOOTUP;
void theme_init() { current_context= THEME_BOOTUP; }


void setcontext(ThemeContext context) {
    current_context = context;
}

ThemeContext getcontext() { return current_context; }

void sbootup_theme(ThemeType type) {
    bootup_theme = (type == THEME_STD) ? &STD_THEME : &FLU_THEME;
}

void sconsole_theme(ThemeType type) {
    console_theme = (type == THEME_STD) ? &STD_THEME : &FLU_THEME;
}

void spanic_theme(ThemeType type) {
    panic_theme = (type == THEME_STD) ? &STD_THEME : &FLU_THEME;
}

u32 get_color(ThemeColor color) {
    const ThemeColors* active_theme;

    switch (current_context) {
        case THEME_BOOTUP:
            active_theme = bootup_theme;
            break;
        case THEME_CONSOLE:
            active_theme = console_theme;
            break;
        case THEME_PANIC:
            active_theme = panic_theme;
            break;
        default: // this is the default theme which will be used when no theme is selected
            active_theme = bootup_theme;
            break;
    }

    switch (color) {
        case COLOR_BLACK:   return active_theme->BLACK;
        case COLOR_BG:      return active_theme->BG;
        case COLOR_RED:     return active_theme->RED;
        case COLOR_GREEN:   return active_theme->GREEN;
        case COLOR_YELLOW:  return active_theme->YELLOW;
        case COLOR_BLUE:    return active_theme->BLUE;
        case COLOR_PURPLE:  return active_theme->PURPLE;
        case COLOR_CYAN:    return active_theme->CYAN;
        case COLOR_WHITE:   return active_theme->WHITE;
        default:            return active_theme->WHITE;
    }
}



u32 black()   { return get_color(COLOR_BLACK); }
u32 bg()      { return get_color(COLOR_BG); }
u32 red()     { return get_color(COLOR_RED); }
u32 green()   { return get_color(COLOR_GREEN); }
u32 yellow()  { return get_color(COLOR_YELLOW); }
u32 blue()    { return get_color(COLOR_BLUE); }
u32 purple()  { return get_color(COLOR_PURPLE); }
u32 cyan()    { return get_color(COLOR_CYAN); }
u32 white()   { return get_color(COLOR_WHITE); }

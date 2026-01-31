#include "theme.h"
#include <kernel/file_systems/vfs/vfs.h>
#include <kernel/data/conf.h>
#include <string/string.h>

// Boot and Panic themes - hardcoded
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

// console
static const ThemeColors DEFAULT_CONSOLE_THEME = {
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
static ThemeColors console_theme_colors;
static const ThemeColors* panic_theme = &STD_THEME;
static ThemeContext current_context = THEME_BOOTUP;
static int console_theme_loaded = 0;

static u32 parse_color_value(const char *value) {
    if (!value) return 0xFFFFFFFF;

    // skip "0x" or "#" prefix
    if (value[0] == '0' && value[1] == 'x') value += 2;
    else if (value[0] == '#') value += 1;

    u32 color = 0;
    for (int i = 0; i < 8 && value[i]; i++) {
        color <<= 4;
        if (value[i] >= '0' && value[i] <= '9') {
            color |= (value[i] - '0');
        } else if (value[i] >= 'A' && value[i] <= 'F') {
            color |= (value[i] - 'A' + 10);
        } else if (value[i] >= 'a' && value[i] <= 'f') {
            color |= (value[i] - 'a' + 10);
        }
    }
    return color;
}

static void load_console_theme_from_config(void) {
    conf_entry_t entries[16];
    int count = conf_load("/.config/ekmsh/theme.cfg", entries, 16);

    if (count <= 0) {
        // uses FLU theme for default
        console_theme_colors = DEFAULT_CONSOLE_THEME;
        console_theme_loaded = 1;
        return;
    }

    // parses each color
    const char *black = conf_get(entries, count, "BLACK");
    const char *bg = conf_get(entries, count, "BG");
    const char *red = conf_get(entries, count, "RED");
    const char *green = conf_get(entries, count, "GREEN");
    const char *yellow = conf_get(entries, count, "YELLOW");
    const char *blue = conf_get(entries, count, "BLUE");
    const char *purple = conf_get(entries, count, "PURPLE");
    const char *cyan = conf_get(entries, count, "CYAN");
    const char *white = conf_get(entries, count, "WHITE");

    console_theme_colors.BLACK = parse_color_value(black ? black : "0xFF111111");
    console_theme_colors.BG = parse_color_value(bg ? bg : "0xFF1F1F1F");
    console_theme_colors.RED = parse_color_value(red ? red : "0xFF9E6E6E");
    console_theme_colors.GREEN = parse_color_value(green ? green : "0xFF7A8A7A");
    console_theme_colors.YELLOW = parse_color_value(yellow ? yellow : "0xFFB8A788");
    console_theme_colors.BLUE = parse_color_value(blue ? blue : "0xFF6E7F8E");
    console_theme_colors.PURPLE = parse_color_value(purple ? purple : "0xFF857A8E");
    console_theme_colors.CYAN = parse_color_value(cyan ? cyan : "0xFF7A8E8E");
    console_theme_colors.WHITE = parse_color_value(white ? white : "0xFFD8D8D8");

    console_theme_loaded = 1;
}

void theme_init() {
    current_context = THEME_BOOTUP;
    console_theme_loaded = 0;
}

void setcontext(ThemeContext context) {
    current_context = context;

    // loads console theme on first use
    if (context == THEME_CONSOLE && !console_theme_loaded) {
        load_console_theme_from_config();
    }
}

ThemeContext getcontext() { return current_context; }

void sbootup_theme(ThemeType type) {
    bootup_theme = (type == THEME_STD) ? &STD_THEME : &FLU_THEME;
}

/*void sconsole_theme(ThemeType type) {
    console_theme = (type == THEME_STD) ? &STD_THEME : &FLU_THEME;
}*/

void spanic_theme(ThemeType type) {
    panic_theme = (type == THEME_STD) ? &STD_THEME : &FLU_THEME;
}

void reload_console_theme(void) {
    console_theme_loaded = 0;
    load_console_theme_from_config();
}

u32 get_color(ThemeColor color) {
    const ThemeColors* active_theme;

    switch (current_context) {
        case THEME_BOOTUP:
            active_theme = bootup_theme;
            break;
        case THEME_CONSOLE:
            //active_theme = console_theme;
            if (!console_theme_loaded) {
                load_console_theme_from_config();
            }
            active_theme = &console_theme_colors;
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

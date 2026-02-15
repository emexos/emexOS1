#ifndef FONTS_H
#define FONTS_H
#include <types.h>

typedef enum {
    FONT_8X8_BOLD = 0,
    FONT_8X8 = 1,
    FONT_8X16 = 2,
    FONT_8X16_BOLD = 3,
    FONT_16X32 = 4,
    //GOHUFONT = 5,
    FONT_COUNT = 5//6
} font_type_t;
typedef struct {
    const char *name;
    const void *data;
    u32 char_width;
    u32 char_height;
} font_t;

// font registry
extern const font_t font_registry[FONT_COUNT];

#endif

#ifndef FONTS_H
#define FONTS_H
#include <types.h>

typedef enum {
    FONT_8X12_BOLD = 0,
    FONT_8X12 = 1,
    FONT_8X8_BOLD = 2,
    FONT_8X8 = 3,
    FONT_8X16 = 4,
    FONT_8X16_BOLD = 5,
    FONT_16X32 = 6,
    //GOHUFONT = 7,
    FONT_COUNT = 7//8
} font_type_t;
typedef struct {
    const char *name;
    const void *data;
    u32 char_width;
    u32 char_height;
    u32 row_bytes;
    u32 lsb_left;
    u32 glyph_count;
    u32 unicode_direct;
} font_t;

// font registry
extern const font_t font_registry[FONT_COUNT];

#endif

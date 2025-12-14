#ifndef FONTS_H
#define FONTS_H
#include <types.h>

typedef enum {
    FONT_8X8_DOS = 0,
    FONT_8X8 = 1,
    FONT_COUNT = 2
} font_type_t;
typedef struct {
    const char *name;
    const u8 (*data)[132][8];
    u32 char_width;
    u32 char_height;
} font_t;

// font registry
extern const font_t font_registry[FONT_COUNT];

#endif

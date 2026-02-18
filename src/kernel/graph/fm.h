#ifndef FM_H
#define FM_H

#include <types.h>
#include <ui/fonts.h>

void fm_init(void);
int f_setcontext(font_type_t font_type);
font_type_t fm_get_current_font(void);
const u8* fm_get_glyph_cp(u32 codepoint);

// get fm properties
u32 fm_get_char_width(void);
u32 fm_get_char_height(void);
const char* fm_get_font_name(void);

#endif

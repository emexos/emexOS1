#pragma once

#include <types.h>
#include <kernel/graph/graphics.h>

typedef enum HorizontalAlignment {
    Leading,
    HorizontalCenter,
    Trailing,
} HorizontalAlignment;

typedef enum VerticalAlignment {
    Top,
    Bottom,
    VerticalCenter,
} VerticalAlignment;

typedef struct text_ui_config {
    const char* text;
    u16 font_size;
} text_ui_config;

typedef struct rect_ui_config {
    u32 border_size;
    u32 border_color;
    u32 border_padding;
} rect_ui_config;

typedef struct common_ui_config {
    struct { u32 x; u32 y; } point;
    struct { u32 width; u32 height; } size;
    HorizontalAlignment horizontal_align;
    VerticalAlignment vertical_align;
    u32 background_color;
    u32 foreground_color;
} common_ui_config;

typedef void* draw_ctx;

#ifdef __cplusplus
extern "C" {
#endif

#define DRAW(item, children)\
({\
common_ui_config parent = item;\
(void)parent;\
children;\
})

#define RELATIVE(a,b) { parent.point.x + a, parent.point.y + b }

common_ui_config ui_label(draw_ctx *ctx, text_ui_config text_config, common_ui_config common_config);
common_ui_config ui_textbox(draw_ctx *ctx, text_ui_config text_config, common_ui_config common_config);
common_ui_config ui_rectangle(draw_ctx *ctx, rect_ui_config rect_config, common_ui_config common_config);

#ifdef __cplusplus
}
#endif

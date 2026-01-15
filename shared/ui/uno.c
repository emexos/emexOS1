#include "uno.h"
#include <string/string.h>
#include <string/print.h>

static u32 calculate_text_width(const char *text, u16 font_size) {
    if (!text) return 0;
    int max_len = 0;
    int current_len = 0;

    for (const char *p = text; *p; p++) {
        if (*p == '\n') {
            if (current_len > max_len) max_len = current_len;
            current_len = 0;
        } else {
            current_len++;
        }
    }
    if (current_len > max_len) max_len = current_len;

    return max_len * (8 * font_size);
}

static u32 calculate_text_height(const char *text, u16 font_size) {
    if (!text) return 0;
    int lines = 1;
    for (const char *p = text; *p; p++) {
        if (*p == '\n') lines++;
    }
    return lines * (8 * font_size + 2);
}

common_ui_config ui_label(draw_ctx *ctx, text_ui_config text_config, common_ui_config common_config) {
    if (!text_config.text || text_config.font_size == 0) return common_config;

    u32 text_w = calculate_text_width(text_config.text, text_config.font_size);
    u32 text_h = calculate_text_height(text_config.text, text_config.font_size);

    u32 x = common_config.point.x;
    u32 y = common_config.point.y;

    // Horizontal alignment
    switch (common_config.horizontal_align) {
        case Trailing:
            x += (common_config.size.width > text_w) ? (common_config.size.width - text_w) : 0;
            break;
        case HorizontalCenter:
            x += (common_config.size.width > text_w) ? ((common_config.size.width - text_w) / 2) : 0;
            break;
        default:
            break;
    }

    // Vertical alignment
    switch (common_config.vertical_align) {
        case Bottom:
            y += (common_config.size.height > text_h) ? (common_config.size.height - text_h) : 0;
            break;
        case VerticalCenter:
            y += (common_config.size.height > text_h) ? ((common_config.size.height - text_h) / 2) : 0;
            break;
        default:
            break;
    }

    // Save cursor position
    u32 old_x = cursor_x;
    u32 old_y = cursor_y;
    u32 old_scale = font_scale;

    cursor_x = x;
    cursor_y = y;
    font_scale = text_config.font_size;

    // Draw text
    string(text_config.text, common_config.foreground_color);

    // Restore cursor and scale
    cursor_x = old_x;
    cursor_y = old_y;
    font_scale = old_scale;

    return common_config;
}

common_ui_config ui_rectangle(draw_ctx *ctx, rect_ui_config rect_config, common_ui_config common_config) {
    u32 bx = common_config.point.x;
    u32 by = common_config.point.y;
    u32 bw = common_config.size.width;
    u32 bh = common_config.size.height;
    u32 b = rect_config.border_size;
    u32 p = rect_config.border_padding;

    // Draw background
    if ((common_config.background_color >> 24) != 0) {
        u32 inner_x = bx + (rect_config.border_padding ? 0 : b);
        u32 inner_y = by + (rect_config.border_padding ? 0 : b);
        u32 twice_b = rect_config.border_padding ? 0 : (b << 1);
        u32 inner_w = (bw > twice_b) ? (bw - twice_b) : 0;
        u32 inner_h = (bh > twice_b) ? (bh - twice_b) : 0;

        if (inner_w && inner_h) {
            draw_rect(inner_x, inner_y, inner_w, inner_h, common_config.background_color);
        }
    }

    // Draw border
    if (b > 0 && (rect_config.border_color >> 24) != 0) {
        u32 twice_p = p << 1;

        // Left border
        draw_rect(bx + p, by + p, b, bh - twice_p, rect_config.border_color);
        // Right border
        draw_rect(bx + bw - b - p, by + p, b, bh - twice_p, rect_config.border_color);
        // Top border
        draw_rect(bx + p, by + p, bw - twice_p, b, rect_config.border_color);
        // Bottom border
        draw_rect(bx + p, by + bh - b - p, bw - twice_p, b, rect_config.border_color);
    }

    return (common_ui_config){
        .point = {bx + p + b, by + p + b},
        .size = {(bw > ((p + b) << 1)) ? (bw - ((p + b) << 1)) : 0,
                 (bh > ((p + b) << 1)) ? (bh - ((p + b) << 1)) : 0},
        .background_color = common_config.background_color,
        .foreground_color = common_config.foreground_color,
        .horizontal_align = common_config.horizontal_align,
        .vertical_align = common_config.vertical_align
    };
}

common_ui_config ui_textbox(draw_ctx *ctx, text_ui_config text_config, common_ui_config common_config) {
    common_ui_config inner = ui_rectangle(ctx, (rect_ui_config){}, common_config);
    ui_label(ctx, text_config, inner);
    return inner;
}

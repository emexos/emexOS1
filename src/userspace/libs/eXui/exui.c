#include "exui.h"

#include <string.h>

#define FH 12  /* font height font8x12*/


/*void exui_begin(exui_ctx_t *ctx) {
    if (!ctx) return;
    ctx->count = 0;
}
*/

/*
void exui_end(exui_ctx_t *ctx) {
    if (!ctx) return;

    desktop.clear();

    //

    desktop.present();
}*/

/*
void exui_label(exui_ctx_t *ctx, int x, int y, const char *text, unsigned int color) {
    if (!ctx || ctx->count >= EXUI_MAX_ELEMS || !text) return;
    exui_elem_t *e = &ctx->elems[ctx->count++];
    e->type  = EXUI_LABEL;
    e->x     = x;
    e->y     = y;
    e->w     = 0;
    e->h     = FH;
    e->color = color;
    _sncopy(e->text, text, EXUI_INPUT_MAX);
    e->focused = 0;
}
*/
/*
void exui_rect(exui_ctx_t *ctx, int x, int y, int w, int h, unsigned int color) {
    if (!ctx || ctx->count >= EXUI_MAX_ELEMS) return;
    exui_elem_t *e = &ctx->elems[ctx->count++];
    e->type  = EXUI_RECT;
    e->x     = x;
    e->y     = y;
    e->w     = w;
    e->h     = h;
    e->color = color;
    e->text[0] = '\0';
    e->focused  = 0;
}
*/
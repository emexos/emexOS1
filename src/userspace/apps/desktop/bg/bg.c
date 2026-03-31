#include "bg.h"
#include "../compositor/comp.h"
#include "../config/cfg.h"

void bg_init(int w, int h) {
	(void)w; (void)h;
}

void bg_draw_full(void)
{
    comp_fill(0, 0, comp_w(), comp_h(), DT_BG);
}

void bg_draw_rect(int x, int y, int w, int h)
{
    comp_fill(x, y, w, h, DT_BG);
}
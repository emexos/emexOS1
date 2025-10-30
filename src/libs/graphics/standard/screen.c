#include "../../../../shared/types.h"
#include "../graphics.h"
#include "../theme/stdclrs.h"
//#include "../draw.h"
#include "../../print/print.h"

void clear(u32 color)
{
    u32 w = get_fb_width();
    u32 h = get_fb_height();

    draw_rect(0, 0, w, h, color);

    reset_cursor();
    print(" ", GFX_BG);
}

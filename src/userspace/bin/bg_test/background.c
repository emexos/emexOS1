#include "gen.h"

void draw_background(void) {
    for (int y = 0; y < scr_h; y++) {
        for (int x = 0; x < scr_w && x < ROW_BUF_W; x++)
        {
		    #if WINDOWS95 == 1
		        row_buf[x] = WIN95_TEAL;
		    #elif BG_MACOS == 1
		        row_buf[x] = checker(x, y);
		    #else
		        row_buf[x] = GRAY_D;
		    #endif
        }
        blit_row(0, y, scr_w, row_buf);
    }
}

void erase_rect(int x, int y, int w, int h) {
    blit_rect_bg(x, y, w, h);
}
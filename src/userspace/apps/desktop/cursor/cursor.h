#pragma once

#define CUR_W 12
#define CUR_H 16

void cur_init(int fb_fd, int w, int h);
void cur_undo_from_backbuf(void);
void cur_bake(int x, int y);
void cur_erase_fb(void);
void cur_draw_fb(int x, int y);

int cur_valid(void);
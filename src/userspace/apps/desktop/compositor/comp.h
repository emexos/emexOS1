#pragma once

unsigned int comp_get(int x, int y);


int comp_w(void);
int comp_h(void);

void comp_init(int fb_fd, int w, int h);
void comp_capture(void);
void comp_fill(int x, int y, int w, int h, unsigned int color);
void comp_set(int x, int y, unsigned int c);
void comp_put_row(int x, int y, const unsigned int *row, int len);
void comp_flush(void);

void comp_copy_rect(
	int src_x, int src_y,
    int dst_x, int dst_y,
    int w, int h
);
void comp_put_pixels(
	int x, int y, int w, int h,
    const unsigned int *pixels
);
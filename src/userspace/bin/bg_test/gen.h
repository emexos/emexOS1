#pragma once

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <emx/fb.h>
#include <emx/mouse.h>
#include <emx/sinfo.h>

// config
// set one to 1
#define BG_MACOS  1
#define WINDOWS95 0
#define BG_EMEX   0


#define BLACK 0xFF000000u
#define WHITE 0xFFFFFFFFu

// macos
#define GRAY_L 0xFFCCCCCCu
#define GRAY_D 0xFF333333u
#define CHECKER_SIZE 2


//win95
#define WIN95_TEAL 0xFF008080u


#define WM_TITLE "information"
#define WM_REG   "/tmp/wm/wm_reg"
#define WM_DIRTY "/tmp/wm/wm_dirty"
#define WIN_W     400
#define WIN_H     260
#define WIN_GAP   9
#define ROW_BUF_W 2048
#define TITLE_H   18 // title bar hight
#define TITLE_P   0
#define TITLE_P_  -1 // bottom line of title bar
#define TITLE_P_B 2 // subtracted with title bar heigh == title y coordinate
#define TITLE_BG  WHITE

#define PAD 8
#define FW  8
#define CW  9
#define CH 12
#define FH 12

extern int fb;
extern int scr_w, scr_h, scr_pitch;
extern int win_x, win_y;
extern unsigned int row_buf[ROW_BUF_W];

void graphics_init(void);
void blit_row(int x, int y, int w, unsigned int *px);
void blit_rect_bg(int x, int y, int w, int h);
unsigned int checker(int x, int y);

void draw_background(void);
void erase_rect(int x, int y, int w, int h);

void draw_window(pid_t pid);
int claim_slot(pid_t pid);
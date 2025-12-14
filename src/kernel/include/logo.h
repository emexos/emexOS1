#ifndef LOGO_H
#define LOGO_H

#include <types.h>

extern u32 logo_width;
extern u32 logo_height;
extern u8 *logo_data;

#define LOGO_WIDTH 371
#define LOGO_HEIGHT 128


void logo_init(void);
void draw_logo(void);
int logo_load_from_fs(const char *path);

extern const char *small_logo_text;
//extern const char *big_logo;

#endif
#pragma once

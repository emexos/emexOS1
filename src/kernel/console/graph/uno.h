#ifndef CONSOLE_UNO_H
#define CONSOLE_UNO_H

#include <types.h>
#include <theme/stdclrs.h>
#include <theme/doccr.h>

#define BANNER_HEIGHT 15
#define BANNER_Y_SPACING 4  // vertical spacing
#define BANNER_BG_COLOR GFX_GRAY_20  // TODO: add borders
#define BANNER_TEXT_COLOR CONSOLESCREEN_COLOR // White text
#define BANNER_UPDATE_INTERVAL 60

void banner_init(void);
void banner_draw(void);
void banner_update_time(void);
void banner_tick(void);  // called from timer
void banner_force_update(void);  // force update
u32 banner_get_height(void);

#endif

/*#include "gen.h"

//mod
#define USERLOCATE "/user/bin/login.elf"

void _userinit(void)
{
    input_pos = 0;
    font_scale = 1;
    input_buffer[0] = '\0';
    char buf[64];

    f_setcontext(FONT_8X8);
    clear(CONSOLESCREEN_BG_COLOR);
    print("\n", white());

    if (!login_authenticate()) {
        hcf();
    }

    //console_config_init();

   // banner_init();

   // console_window_init();
    //cursor_();+
    // console_start();
}
*/

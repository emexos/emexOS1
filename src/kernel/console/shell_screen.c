#include "console.h"
#include "../../klib/string/print.h"
#include "../../klib/graphics/graphics.h"
#include "../../../shared/theme/stdclrs.h"

void shell_clear_screen(u32 color)
{
    clear(color);
}

void shell_print_prompt(void)
{
    // emexOS will be the directory if we have a file system
    string("emexOS:~> ", GFX_GREEN);
}

void shell_redraw_input(void)
{
    // for future use: redraw current input line
    // useful when implementing line editing features
}

#include "console.h"

void shell_clear_screen(u32 color)
{
    clear(color);
}

void shell_print_prompt(void)
{
    // emexOS will be the directory if we have a file system

    // in future it should be like:
    /*
     * string("\n", GFX_WHITE);
     * string(variable, GFX_PURPLE); // variable is the current dir
     * string(" > ", GFX_WHITE);
     */
    //

    string("\n", GFX_WHITE);
    string("\x01 ", GFX_YELLOW);
}

void shell_redraw_input(void)
{
    // for future use: redraw current input line
    // useful when implementing line editing features (programms)
}

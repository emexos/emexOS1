#include "console.h"
//#include "graph/uno.h"
//#include "graph/dos.h"
#include <kernel/data/conf/conf.h>

void shell_clear_screen(u32 color)
{

    console_window_clear(color);
}

static conf_entry_t prompt_conf[16];
static int prompt_conf_count = 0;

void shell_load_prompt_config(void) {
    prompt_conf_count = conf_load("/.config/ekmsh/prompt.cfg",
                                   prompt_conf, 16);
}

extern char cwd[];

/*void shell_print_prompt(void)
{

    string("\n", GFX_WHITE);
    string(user_config_get_pc_name(), GFX_WHITE);
    string("@", GFX_WHITE);
    string(user_config_get_user_name(), GFX_WHITE);
    //string("\x01 ", GFX_YELLOW);
    string(":", GFX_WHITE);
    if (str_len(cwd) > 1 && cwd[str_len(cwd) - 1] == '/') {
        char prompt_cwd[MAX_PATH_LEN];
        str_copy(prompt_cwd, cwd);
        prompt_cwd[str_len(cwd) - 1] = '\0';
        string(prompt_cwd, GFX_WHITE);
    }
    string("# ", GFX_BLUE);
}*/

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

    if (prompt_conf_count <= 0) {
        shell_load_prompt_config();
    }

    const char *format = conf_get(prompt_conf, prompt_conf_count, "format");

    if (!format) {
        //printf("using standard");
        cprintf("\n[", GFX_WHITE);
        cprintf(uci_get_pc_name(), GFX_WHITE);
        cprintf("@", GFX_WHITE);
        cprintf(uci_get_user_name(), GFX_WHITE);
        cprintf("]", white());

        if (str_len(cwd) > 1 && cwd[str_len(cwd) - 1] == '/') {
            char prompt_cwd[MAX_PATH_LEN];
            str_copy(prompt_cwd, cwd);
            prompt_cwd[str_len(cwd) - 1] = '\0';
            cprintf(prompt_cwd, white());
        } else {
            cprintf(cwd, white());
        }

        cprintf("# ", blue());
        return;
    }

    cprintf("\n", white());

    for (const char *p = format; *p; p++) {
        if (*p == '%') {
            p++;
            if (*p == 'u') cprintf(uci_get_user_name(), white());
            else if (*p == 'h') cprintf(uci_get_pc_name(), white());
            else if (*p == 'w') {
                if (str_len(cwd) > 1 && cwd[str_len(cwd) - 1] == '/') {
                    char prompt_cwd[MAX_PATH_LEN];
                    str_copy(prompt_cwd, cwd);
                    prompt_cwd[str_len(cwd) - 1] = '\0';
                    cprintf(prompt_cwd, white());
                } else {
                    cprintf(cwd, white());
                }
            }
            else if (*p == '%') cprintf("%", white());
        } else {
            char c[2] = {*p, '\0'};
            cprintf(c, white());
        }
    }
}

void shell_redraw_input(void)
{
    // for future use: redraw current input line
    // useful when implementing line editing features (programms)
}

// src/kernel/console/functions/source.c
#include <kernel/console/console.h>

FHDR(cmd_source) {

    // reloads the shell

    if (!s || *s == '\0') {
        print("usage: source <file>\n", GFX_RED);
        return;
    }

    if (str_equals(s, "console") || str_equals(s, "console/")) {
        user_config_reload();

        banner_force_update();

        clear(CONSOLESCREEN_BG_COLOR);
        banner_draw();
        console_window_init();

        print("Console reloaded\n", GFX_GREEN);
        shell_print_prompt();
        return;
    }

    print("source: only 'console' is supported\n", GFX_RED);
}

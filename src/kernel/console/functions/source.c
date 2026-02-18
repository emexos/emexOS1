// src/kernel/console/functions/source.c
#include <kernel/console/console.h>

FHDR(cmd_source) {

    // reloads the shell

    if (!s || *s == '\0') {
        cprintf("usage: source <file>\n", GFX_RED);
        return;
    }

    if (str_equals(s, "console") || str_equals(s, "console/")) {
        uci_reload();

        reload_console_theme();

        // Redraw banner
        banner_force_update();

        clear(CONSOLESCREEN_BG_COLOR);
        banner_draw();
        console_window_init();

        cprintf("Console reloaded\n", GFX_GREEN);
        //shell_print_prompt();
        return;
    }

    cprintf("source: only 'console' is supported\n", GFX_RED);
}

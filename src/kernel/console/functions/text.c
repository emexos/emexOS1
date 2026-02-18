#include <kernel/console/console.h>

FHDR(cmd_echo)
{
    if (*s == '\0') {
        cprintf("\n", GFX_WHITE);
        return;
    }

    cprintf(s, GFX_WHITE);
}

FHDR(cmd_clear)
{
    u32 color = CONSOLESCREEN_BG_COLOR;

    /*
    if (*s != '\0') {
        if (!parse_color(s, &color)) {
            cprintf("wrong color\n", GFX_RED);
            return;
        }
    }
    */

    if (*s != '\0') {
        return;
    }

    shell_clear_screen(color);
    banner_force_update();
}

FHDR(cmd_fsize)
{
    if (*s == '\0') {
        char buf[64];
        str_copy(buf, "Current font size: ");
        str_append_uint(buf, font_scale);
        cprintf(buf, GFX_WHITE);
        return;
    }

    // skip spaces
    while (*s == ' ') s++;

    // parse number
    int size = 0;
    while (*s >= '0' && *s <= '9') {
        size = size * 10 + (*s - '0');
        s++;
    }

    if (size < 1 || size > 4) {
        cprintf("Invalid size. Use 1-4\n", GFX_RED);
        return;
    }

    clear(CONSOLESCREEN_BG_COLOR);
    set_font_scale(size);
    /*char buf[64];
    str_copy(buf, " Font size set to ");
    str_append_uint(buf, size);
    cprintf(buf, GFX_GREEN);
    */
    clear(CONSOLESCREEN_BG_COLOR);
    banner_force_update();
    console_window_update_layout();

    cursor_x = CONSOLE_PADDING_X;
    cursor_y = banner_get_height();
}

FHDR(cmd_font) {
    if (*s == '\0') {
        return;
    }

    // Parse font number
    const char *p = s;
    while (*p == ' ') p++;

    int font_num = 0;
    while (*p >= '0' && *p <= '9') {
        font_num = font_num * 10 + (*p - '0');
        p++;
    }

    if (f_setcontext((font_type_t)font_num) == 0) {
        clear(CONSOLESCREEN_BG_COLOR);
        banner_force_update();
        cprintf("\n", GFX_GREEN);
    } else {
        cprintf("use: font <number>", GFX_RED);
    }
}

FHDR(cmd_cyrillc_sumbol)
{
    (void)s;
    cprintf("АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ\n", GFX_WHITE);
    cprintf("абвгдеёжзийклмнопрстуфхцчшщъыьэюя\n", GFX_WHITE);
}

FHDR(cmd_latin_sumbol)
{
    (void)s;
    cprintf("ABCDEFGHIJKLMNOPQRSTUVWXYZ\n", GFX_WHITE);
    cprintf("abcdefghijklmnopqrstuvwxyz\n", GFX_WHITE);
}

FHDR(cmd_all_sumbol)
{
    (void)s;
    cprintf("ABCDEFGHIJKLMNOPQRSTUVWXYZ\n", GFX_WHITE);
    cprintf("abcdefghijklmnopqrstuvwxyz\n", GFX_WHITE);
    cprintf("AEOEUESS: ÄÖÜ äöü ß\n", GFX_WHITE);
    cprintf("АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ\n", GFX_WHITE);
    cprintf("абвгдеёжзийклмнопрстуфхцчшщъыьэюя\n", GFX_WHITE);
    cprintf("0123456789 !?.,:;+-*/=_()[]{}@#%&$°^\"'<>\\|~`§€\n", GFX_WHITE);
    cprintf("«∑€®†Ω¨⁄øπ•±å‚∂ƒ©ªº∆@œæ‘≤¥≈ç√∫~µ∞…–¡“¶¢≠¿'£\n", GFX_WHITE);
}

/*FHDR(cmd_help)
{
    if (*s == '\0') {
        // show all commands
        cprintf("[COMMMON]\n", GFX_YELLOW);
        cprintf("  echo [text]    - echo [text]\n", GFX_WHITE);
        cprintf("  clear [color]  - clear screen\n", GFX_WHITE);
        cprintf("  help [command] - displays this list\n", GFX_WHITE);
        cprintf("  scale [1-4]    - change screen size\n", GFX_WHITE);
        cprintf("  font [0-1]     - switch fonts\n", GFX_WHITE);
        cprintf("  date           - show current date\n", GFX_WHITE);
        cprintf("  time           - show current time\n", GFX_WHITE);
        cprintf("  calendar       - show date & time\n", GFX_WHITE);
        cprintf("  uptime         - displays the uptime\n", GFX_WHITE);
        cprintf("  dofetch        - emexOS system fetch\n", GFX_WHITE);
        cprintf("  cat <file>     - show file content\n", GFX_WHITE);
        cprintf("  ls <path>      - list directory contents\n", GFX_WHITE);
        cprintf("  shut           - shuts down the system\n", GFX_WHITE);
        //cprintf("[SYSTEM]\n", GFX_YELLOW);
        cprintf("  meminfo        - heap memory information\n", GFX_WHITE);
        cprintf("  modules        - shows all modules in fs\n", GFX_WHITE);
        cprintf("Type 'help <command>' for details", GFX_GRAY_50);
    } else {
        // show specific command help
        const char *p = s;
        while (*p == ' ') p++;

        console_cmd_t *cmd = console_find_cmd(p);
        if (cmd) {
            char buf[128];

            str_copy(buf, "\n");
            str_append(buf, cmd->description);
            cprintf(buf, GFX_WHITE);

            str_copy(buf, "\nUsage: ");
            str_append(buf, cmd->usage);
            cprintf(buf, GFX_YELLOW);
            cprintf("\n", GFX_WHITE);
        } else {
            cprintf("\nCommand not found", GFX_RED);
        }
    }
}
*/

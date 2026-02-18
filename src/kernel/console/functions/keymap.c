#include <kernel/console/console.h>
#include <drivers/ps2/keyboard/maps.h>

FHDR(cmd_keymap) {
    if (!s || *s == '\0') {
        // Show current keymap
        cprintf("you use keymap: ", GFX_WHITE);
        cprintf(keymap_get_current_name(), GFX_WHITE);
        cprintf("\nuse: keymap <name>\n", GFX_WHITE);
        cprintf("\nnote that only DE & US are correctly supported by now.\n", GFX_WHITE);
        return;
    }

    // parse keymap name
    const char *p = s;
    while (*p == ' ') p++;

    char keymap_name[16];
    int i = 0;
    while (*p && *p != ' ' && *p != '\n' && i < 15) {
        keymap_name[i++] = *p++;
    }
    keymap_name[i] = '\0';

    // convert to uppercase
    str_to_upper(keymap_name);

    // trys to set the keymap
    if (keymap_set(keymap_name) != 0) {
        cprintf("error: keymap '", GFX_RED);
        cprintf(keymap_name, GFX_RED);
        cprintf("' not found\n", GFX_RED);
        cprintf("try: US, DE\n", GFX_RED);
        return;
    }

    cprintf("keymap changed to: ", GFX_WHITE);
    cprintf(keymap_name, GFX_WHITE);
    cprintf("\n", GFX_WHITE);
}

#include <kernel/console/console.h>
#include <drivers/ps2/keyboard/maps.h>

FHDR(cmd_keymap) {
    if (!s || *s == '\0') {
        // Show current keymap
        print("you use keymap: ", GFX_WHITE);
        print(keymap_get_current_name(), GFX_WHITE);
        print("\nuse: keymap <name>\n", GFX_WHITE);
        print("\nnote that only DE & US are correctly supported by now.\n", GFX_WHITE);
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
        print("error: keymap '", GFX_RED);
        print(keymap_name, GFX_RED);
        print("' not found\n", GFX_RED);
        print("try: US, DE\n", GFX_RED);
        return;
    }

    print("keymap changed to: ", GFX_WHITE);
    print(keymap_name, GFX_WHITE);
    print("\n", GFX_WHITE);
}

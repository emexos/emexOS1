#include "maps.h"
#include "loader.h"
#include <kernel/file_systems/vfs/vfs.h>
#include <memory/main.h>
#include <string/string.h>
#include <kernel/communication/serial.h>
#include <config/files.h>

static keymap_t current_keymap;
static char current_keymap_name[16] = "US";

int keymap_init(void) {
    log("[KEYMAP]", "Initializing keymap system...\n", d);
    keymap_modules_init();

    // loads default (US)
    if (keymap_load_from_module("US", &current_keymap) == 0) {
        str_copy(current_keymap_name, "US");
        log("[KEYMAP]", "Loaded default: US\n", d);
        return 0;
    }

    // If US not found, try DE
    if (keymap_load_from_module("DE", &current_keymap) == 0) {
        str_copy(current_keymap_name, "DE");
        log("[KEYMAP]", "Loaded default: DE\n", d);
        return 0;
    }

    if (keymap_load_from_module("PL", &current_keymap) == 0) {
        str_copy(current_keymap_name, "PL");
        log("[KEYMAP]", "Loaded default: PL\n", d);
        return 0;
    }

    if (keymap_load_from_module("RU", &current_keymap) == 0) {
        str_copy(current_keymap_name, "RU");
        log("[KEYMAP]", "Loaded default: RU\n", d);
        return 0;
    }

    log("[KEYMAP]", "ERROR: No keymap could be loaded!\n", warning);
    return -1;
}

const keymap_t* keymap_get_current(void) {
    return &current_keymap;
}

const char* keymap_get_current_name(void) {
    return current_keymap_name;
}

int keymap_set(const char *name) {
    if (!name) return -1;

    keymap_t new_keymap;

    if (keymap_load_from_module(name, &new_keymap) == 0) {
        memcpy(&current_keymap, &new_keymap, sizeof(keymap_t));
        str_copy(current_keymap_name, name);

        printf("[KEYMAP] Switched to: %s\n", name);
        return 0;
    }

    printf("ERROR: Keymap '%s' not found\n", name);
    return -1;
}

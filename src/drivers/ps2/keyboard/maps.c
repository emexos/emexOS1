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
    printf("[KEYMAP] Initializing keymap system...\n");

    // First, list all available keymaps from Limine modules
    keymap_modules_init();

    // Try to load default keymap (US)
    if (keymap_load_from_module("US", &current_keymap) == 0) {
        str_copy(current_keymap_name, "US");
        printf("[KEYMAP] Loaded default: US\n");
        return 0;
    }

    // If US not found, try DE
    if (keymap_load_from_module("DE", &current_keymap) == 0) {
        str_copy(current_keymap_name, "DE");
        printf("[KEYMAP] Loaded default: DE\n");
        return 0;
    }

    printf("[KEYMAP] ERROR: No keymap could be loaded!\n");
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

    // Try to load from Limine module first
    if (keymap_load_from_module(name, &new_keymap) == 0) {
        // Success - apply new keymap
        memcpy(&current_keymap, &new_keymap, sizeof(keymap_t));
        str_copy(current_keymap_name, name);

        printf("[KEYMAP] Switched to: %s\n", name);
        return 0;
    }

    printf("[KEYMAP] ERROR: Keymap '%s' not found\n", name);
    return -1;
}

#include "module.h"
#include <klib/string/string.h>
#include <theme/stdclrs.h>
#include <klib/graphics/theme.h>
#include <theme/doccr.h>

static driver_module *modules[MAX_MODULES];
static int module_count = 0;

void module_init(void) {
    BOOTUP_PRINT("[MOD] ", GFX_GRAY_70);
    BOOTUP_PRINT("init module system\n", GFX_ST_WHITE);
    for (int i = 0; i < MAX_MODULES; i++) {
        modules[i] = NULL;
    }
    module_count = 0;
}

int module_register(driver_module *module) {
    if (!module || module_count >= MAX_MODULES) {
        return -1;
    }

    // check if already registered
    for (int i = 0; i < module_count; i++) {
        if (modules[i] == module) {
            return -1;
        }
    }

    // call init if exists
    if (module->init) {
        int ret = module->init();
        if (ret != 0) {
            return ret;
        }
    }

    modules[module_count++] = module;
    return 0;
}

void module_unregister(const char *name) {
    if (!name) return;

    for (int i = 0; i < module_count; i++) {
        if (modules[i] && modules[i]->name) {
            // simple string compare
            const char *a = name;
            const char *b = modules[i]->name;
            int match = 1;

            while (*a && *b) {
                if (*a != *b) {
                    match = 0;
                    break;
                }
                a++;
                b++;
            }

            if (match && *a == '\0' && *b == '\0') {
                // call cleanup if exists
                if (modules[i]->fini) {
                    modules[i]->fini();
                }

                // shift array
                for (int j = i; j < module_count - 1; j++) {
                    modules[j] = modules[j + 1];
                }
                modules[module_count - 1] = NULL;
                module_count--;
                return;
            }
        }
    }
}

driver_module* module_find(const char *name) {
    if (!name) return NULL;

    for (int i = 0; i < module_count; i++) {
        if (modules[i] && modules[i]->name) {
            // simple string compare
            const char *a = name;
            const char *b = modules[i]->name;
            int match = 1;

            while (*a && *b) {
                if (*a != *b) {
                    match = 0;
                    break;
                }
                a++;
                b++;
            }

            if (match && *a == '\0' && *b == '\0') {
                return modules[i];
            }
        }
    }
    return NULL;
}

int module_get_count(void) {
    return module_count;
}

driver_module* module_get_by_index(int idx) {
    if (idx < 0 || idx >= module_count) {
        return NULL;
    }
    return modules[idx];
}

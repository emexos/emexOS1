#include "console.h"
#include "../module/module.h"

static int console_module_init(void) {
    // console already initialized in main
    return 0;
}

static void console_module_fini(void) {
    //
}

driver_module_t console_module = {
    .name = "console",
    .mount = "/dev/console",
    .version = VERSION_NUM(0, 1, 0, 0),
    .init = console_module_init,
    .fini = console_module_fini,
    .open = NULL,
    .read = NULL,
    .write = NULL,
};

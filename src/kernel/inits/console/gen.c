#include "gen.h"

#include <kernel/console/console.h>

void console_init_gen(void) {
    promptdirs();
    promptcont();
    init_consoletheme();
    running_console_config_init();
}

#include "gen.h"

//#include <kernel/console/console.h>

char cwd[MAX_PATH_LEN] = "/";

void console_init_gen(void) {
    promptdirs();
    promptcont();
    init_consoletheme();
    running_console_config_init();
}

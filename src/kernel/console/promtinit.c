#include "console.h"
#include <kernel/file_systems/vfs/vfs.h>

// i wont delete this function due to compatibility reasons
void console_config_init(void) {
    console_init_gen();
}

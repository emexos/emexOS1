#include "console.h"
#include <kernel/file_systems/vfs/vfs.h>

void prompt_config_init(void) {
    // Create directories
    fs_mkdir("/.config/ekmsh");
    //fs_mkdir("/.config/ekmsh/prompts");

    // Create default prompt.conf
    int fd = fs_open("/.config/ekmsh/prompt.cfg", O_CREAT | O_WRONLY);
    if (fd < 0) return;

    const char *default_config =
        "# currently there is no color support for the prompt, but soon!\n"
        "# %u = username,\n# %h = hostname,\n# %w = curent directory\n"
        "format: %u@%h:%w# \n";

    fs_write(fd, default_config, str_len(default_config));
    fs_close(fd);
}

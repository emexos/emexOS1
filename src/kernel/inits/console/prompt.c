#include "gen.h"

#include <kernel/console/console.h>


void promptdirs(void) {
    fs_mkdir(CONSOLECONFIG);
    fs_mkdir(CONSOLECONFIG "/fonts");

    //return;
}

void promptcont(void) {
    int fd = fs_open(PROMPT_PATH, O_CREAT | O_WRONLY);
    if (fd >= 0) {
        const char *default_config =
            "# currently there is no color support for the prompt, but soon!\n"
            "# %u = username,\n# %h = hostname,\n# %w = curent directory\n"
            "[%u@%h:%w]# \n"
        ;
        fs_write(fd, default_config, str_len(default_config));
        fs_close(fd);
    }
}

#include "gen.h"

#include <kernel/console/console.h>

void running_console_config_init(void) {
    int fd = fs_open("/.config/ekmsh/con.cfg", O_CREAT | O_WRONLY); // get read by graph/theme
    if (fd >= 0) {
        const char *Cconfiguration =
            "CONSOLE_PROG: \"ekmsh\"\n"
            "CONSOLE_NAME: \"console\"\n"
            "\n"
            "CONSOLE_VERS: \"1.3\" # new with login\n"
        ;

        fs_write(fd, Cconfiguration, str_len(Cconfiguration));
        fs_close(fd);
    }
}

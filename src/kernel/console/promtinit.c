#include "console.h"
#include <kernel/file_systems/vfs/vfs.h>

void prompt_config_init(void) {
    // Create directories
    fs_mkdir("/.config/ekmsh");
    //fs_mkdir("/.config/ekmsh/prompts");

    // Create default prompt.conf
    int fd = fs_open("/.config/ekmsh/prompt.cfg", O_CREAT | O_WRONLY);
    if (fd >= 0) {
        const char *default_config =
            "# currently there is no color support for the prompt, but soon!\n"
            "# %u = username,\n# %h = hostname,\n# %w = curent directory\n"
            "format: %u@%h:%w# \n";

        fs_write(fd, default_config, str_len(default_config));
        fs_close(fd);
    }

    // Create default theme.cfg
    fd = fs_open("/.config/ekmsh/theme.cfg", O_CREAT | O_WRONLY); // get read by graph/theme
    if (fd >= 0) {
        const char *default_theme =
            "# use: 0xAARRGGBB\n"
            "\n"
            "BLACK: 0xFF111111\n"
            "Bd: 0xFF1F1F1F\n"
            "RED: 0xFF9E6E6E\n"
            "GREEN: 0xFF7A8A7A\n"
            "YELLOW: 0xFFB8A788\n"
            "BLUE: 0xFF6E7F8E\n"
            "PURPLE: 0xFF857A8E\n"
            "CYAN: 0xFF7A8E8E\n"
            "WHITE: 0xFFD8D8D8\n";

        fs_write(fd, default_theme, str_len(default_theme));
        fs_close(fd);
    }
}

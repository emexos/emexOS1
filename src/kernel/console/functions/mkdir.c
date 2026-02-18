#include <kernel/console/console.h>
#include <kernel/file_systems/vfs/vfs.h>

extern char cwd[];

FHDR(cmd_mkdir) {
    if (!s || *s == '\0') {
        cprintf("usage: mkdir <directory>\n", GFX_RED);
        return;
    }

    char full_path[MAX_PATH_LEN];

    // Handle absolute vs relative paths
    if (s[0] == '/') {
        str_copy(full_path, s);
    } else {
        str_copy(full_path, cwd);

        if (cwd[str_len(cwd) - 1] != '/') {
            str_append(full_path, "/");
        }

        str_append(full_path, s);
    }

    // Create the directory
    if (fs_mkdir(full_path) != 0) {
        cprintf("error: could not create directory\n", GFX_RED);
        return;
    }

    cprintf("directory created: ", GFX_GREEN);
    cprintf(full_path, GFX_WHITE);
    cprintf("\n", GFX_WHITE);
}

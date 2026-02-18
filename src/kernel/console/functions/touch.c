#include <kernel/console/console.h>
#include <kernel/file_systems/vfs/vfs.h>

extern char cwd[];

FHDR(cmd_touch)
{
    if (!s || *s == '\0') {
        cprintf("usage: touch <file>\n", GFX_RED);
        return;
    }

    char path[MAX_PATH_LEN];

    if (s[0] == '/') {
        str_copy(path, s);
    } else {
        str_copy(path, cwd);
        if (cwd[str_len(cwd) - 1] != '/') str_append(path, "/");
        str_append(path, s);
    }

    int fd = fs_open(path, O_CREAT | O_WRONLY);
    if (fd < 0) {
        cprintf("error: cannot create file\n", GFX_RED);
        return;
    }

    fs_close(fd);
}

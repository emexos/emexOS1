#include <kernel/console/console.h>
#include <kernel/file_systems/vfs/vfs.h>

extern char cwd[];

FHDR(cmd_cat) {
    (void)s;
    if (!s || *s == '\0') {
        cprintf("error: no file specified\n", GFX_RED);
        return;
    }

    char path[MAX_PATH_LEN];

    if (s[0] == '/') {
        str_copy(path, s);
    } else {
        str_copy(path, cwd);

        if (cwd[str_len(cwd) - 1] != '/') {
            str_append(path, "/");
        }

        str_append(path, s);
    }

    // opens in read only (RDONLY)
    int fd = fs_open(path, O_RDONLY);
    if (fd < 0) {
        cprintf("error: cannot open file\n", GFX_RED);
        return;
    }

    char buf[256];
    ssize_t bytes;

    while ((bytes = fs_read(fd, buf, sizeof(buf) - 1)) > 0) {
        buf[bytes] = '\0';
        cprintf(buf, GFX_WHITE);
    }

    fs_close(fd);

   // buf[0] = '\0';
    cprintf("\n", GFX_WHITE);
}

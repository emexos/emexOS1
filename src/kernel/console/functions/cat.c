#include <kernel/console/console.h>
#include <kernel/file_systems/vfs/vfs.h>

//extern char cwd[];

FHDR(cmd_cat) {
    (void)s;

    // opens in read only (RDONLY)
    int fd = fs_open(s, O_RDONLY);
    if (fd < 0) {
        print("error: cannot open file\n", GFX_RED);
        return;
    }

    char buf[256];
    ssize_t bytes;

    while ((bytes = fs_read(fd, buf, sizeof(buf) - 1)) > 0) {
        buf[bytes] = '\0';
        print(buf, GFX_WHITE);
    }

    fs_close(fd);

    buf[0] = '\0';
    print("\n", GFX_WHITE);
}

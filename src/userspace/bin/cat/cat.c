#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define CAT_BUFSZ 4096

static void resolve_path(const char *arg, char *out_buf, int bufsz)
{
	char cwd[256];
	int ci = 0;
	int ai = 0;

    if (!arg || arg[0] == '/') {
        int i = 0;
        if (arg) while (i < bufsz - 1 && arg[i]) { out_buf[i] = arg[i]; i++; }
        out_buf[i] = '\0';
        return;
    }
    if (!getcwd(cwd, sizeof(cwd))) {
        cwd[0] = '/'; cwd[1] = '\0';
    }


    while (ci < bufsz - 1 && cwd[ci]) { out_buf[ci] = cwd[ci]; ci++; }

    if (ci > 0 && out_buf[ci - 1] != '/') {
        if (ci < bufsz - 1) out_buf[ci++] = '/';
    }
    while (ci < bufsz - 1 && arg[ai]) { out_buf[ci++] = arg[ai++]; }
    out_buf[ci] = '\0';
}

static int cat_fd(int src_fd, int dst_fd) {
    char buf[CAT_BUFSZ];
    ssize_t n;
    while ((n = read(src_fd, buf, sizeof(buf))) > 0) {
        ssize_t written = 0;
        while (written < n) {
            ssize_t w = write(dst_fd, buf + written, (size_t)(n - written));
            if (w <= 0) return -1;
            written += w;
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    const char *src_files[32];
    int src_count = 0;
    const char *dst_path = NULL; // NULL == stdout
    int expect_dst = 0;

    for (int i = 1; i < argc; i++) {
        if (expect_dst) {
            dst_path   = argv[i];
            expect_dst = 0;
            continue;
        }

        if (argv[i][0] == '>' && argv[i][1] == '\0') {
            // standalone '>'
            expect_dst = 1;
            continue;
        }

        if (argv[i][0] == '>') {
            // '>foo' destination
            dst_path = argv[i] + 1;
            continue;
        }

        if (src_count < 32)
            src_files[src_count++] = argv[i];
    }

    int dst_fd = STDOUT_FILENO;
    int dst_opened = 0;

    if (dst_path) {
        char dst_resolved[256];
        resolve_path(dst_path, dst_resolved, sizeof(dst_resolved));

        dst_fd = open(dst_resolved, O_WRONLY | O_CREAT);
        if (dst_fd < 0) {
            write(STDERR_FILENO, "cat: cannot open output: ", 25);
            write(STDERR_FILENO, dst_resolved, strlen(dst_resolved));
            write(STDERR_FILENO, "\n", 1);
            return 1;
        }
        dst_opened = 1;
    }
    if (src_count == 0) {
        cat_fd(STDIN_FILENO, dst_fd);
        if (dst_opened) close(dst_fd);
        return 0;
    }

    int ret = 0;

    for (int i = 0; i < src_count; i++) {
        char path[256];
        resolve_path(src_files[i], path, sizeof(path));

        int src_fd = open(path, O_RDONLY);
        if (src_fd < 0) {
            write(STDERR_FILENO, "cat: ", 5);
            write(STDERR_FILENO, path, strlen(path));
            write(STDERR_FILENO, ": cannot open\n", 14);
            ret = 1;
            continue;
        }

        if (cat_fd(src_fd, dst_fd) < 0) {
            write(STDERR_FILENO, "cat: ", 5);
            write(STDERR_FILENO, path, strlen(path));
            write(STDERR_FILENO, ": read/write error\n", 19);
            ret = 1;
        }

        close(src_fd);
    }

    if (dst_opened) close(dst_fd);
    return ret;
}
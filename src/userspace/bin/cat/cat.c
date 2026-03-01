#include <stdio.h>
#include <unistd.h>
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

int main(int argc, char *argv[])
{
    if (argc < 2) {
        // no file given
        // read from stdin and echo to stdout
        char buf[CAT_BUFSZ];
        ssize_t n;
        while ((n = read(STDIN_FILENO, buf, sizeof(buf))) > 0)
            write(STDOUT_FILENO, buf, (size_t)n);
        return 0;
    }

    int ret = 0;

    for (int i = 1; i < argc; i++) {
        char path[256];
        resolve_path(argv[i], path, sizeof(path));

        FILE *f = fopen(path, "rb");
        if (!f) {
            fprintf(stderr, "cat: %s: cannot open\n", path);
            ret = 1;
            continue;
        }

        char buf[CAT_BUFSZ];
        size_t n;

        // fread returns full elements
        while ((n = fread(buf, 1, sizeof(buf), f)) > 0)
            write(STDOUT_FILENO, buf, n);

        if (ferror(f)) {
            fprintf(stderr, "cat: %s: read error\n", path);
            ret = 1;
        }

        fclose(f);
    }

    return ret;
}
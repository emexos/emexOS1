#include <unistd.h>
#include <stdio.h>
#include <string.h>

static void resolve_path(const char *arg, char *out, int bufsz)
{
    if (!arg || arg[0] == '/') {
        int i = 0;
        if (arg) while (i < bufsz - 1 && arg[i]) { out[i] = arg[i]; i++; }
        out[i] = '\0';
        return;
    }
    char cwd[256];
    if (!getcwd(cwd, sizeof(cwd))) { cwd[0] = '/'; cwd[1] = '\0'; }
    int ci = 0;
    while (ci < bufsz - 1 && cwd[ci]) { out[ci] = cwd[ci]; ci++; }
    if (ci > 0 && out[ci - 1] != '/') { if (ci < bufsz - 1) out[ci++] = '/'; }
    int ai = 0;
    while (ci < bufsz - 1 && arg[ai]) { out[ci++] = arg[ai++]; }
    out[ci] = '\0';
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "usage: rm <file> [file ...]\n");
        return 1;
    }

    int ret = 0;
    for (int i = 1; i < argc; i++) {
        char path[256];
        resolve_path(argv[i], path, sizeof(path));
        if (unlink(path) != 0) {
            fprintf(stderr, "rm: cannot remove '%s'\n", path);
            ret = 1;
        }
    }
    return ret;
}
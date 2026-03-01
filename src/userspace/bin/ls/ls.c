#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>

#define COLS 6  // items per row

static void resolve_path(const char *arg, char *out_buf, int bufsz)
{
    if (arg && arg[0] == '/') {
        int i = 0;
        while (i < bufsz - 1 && arg[i]) { out_buf[i] = arg[i]; i++; }
        out_buf[i] = '\0';
        return;
    }

    char cwd[256];
    if (!getcwd(cwd, sizeof(cwd))) {
        cwd[0] = '/'; cwd[1] = '\0';
    }

    if (!arg || arg[0] == '\0') {
        int i = 0;
        while (i < bufsz - 1 && cwd[i]) { out_buf[i] = cwd[i]; i++; }
        out_buf[i] = '\0';
        return;
    }
    int ci = 0;
    while (ci < bufsz - 1 && cwd[ci]) { out_buf[ci] = cwd[ci]; ci++; }
    if (ci > 0 && out_buf[ci - 1] != '/') {
        if (ci < bufsz - 1) out_buf[ci++] = '/';
    }
    int ai = 0;
    while (ci < bufsz - 1 && arg[ai]) { out_buf[ci++] = arg[ai++]; }
    out_buf[ci] = '\0';
}

int main(int argc, char **argv)
{
    char path[256];
    resolve_path((argc > 1) ? argv[1] : NULL, path, sizeof(path));

    DIR *dir = opendir(path);
    if (!dir) {
        write(STDOUT_FILENO, "ls: cannot open '", 17);
        write(STDOUT_FILENO, path, strlen(path));
        write(STDOUT_FILENO, "'\n", 2);
        return 1;
    }

    #define MAX_ENTRIES 256
    #define NAME_MAX_LEN 64

    static char names[MAX_ENTRIES][NAME_MAX_LEN + 1];
    static char is_dir[MAX_ENTRIES];
    int count = 0;

    struct dirent *ent;
    while (count < MAX_ENTRIES && (ent = readdir(dir)) != NULL) {
        int ni = 0;
        const char *src = ent->d_name;
        while (ni < NAME_MAX_LEN && src[ni]) {
            names[count][ni] = src[ni];
            ni++;
        }
        names[count][ni] = '\0';
        is_dir[count] = (ent->d_type == DT_DIR) ? 1 : 0;
        count++;
    }

    closedir(dir);

    // print COLS items per row
    for (int i = 0; i < count; i++) {
        if (is_dir[i]) {
            write(STDOUT_FILENO, "\033[34m", 5);   // blue for dirs
            write(STDOUT_FILENO, names[i], strlen(names[i]));
            write(STDOUT_FILENO, "/", 1);
            write(STDOUT_FILENO, "\033[0m", 4);
        } else {
            write(STDOUT_FILENO, names[i], strlen(names[i]));
        }
        if ((i + 1) % COLS == 0 || i == count - 1) {
            write(STDOUT_FILENO, "\n", 1);
        } else {
            write(STDOUT_FILENO, "\t", 1);
        }
    }

    return 0;
}
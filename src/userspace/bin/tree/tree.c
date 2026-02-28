#include <unistd.h>
#include <dirent.h>
#include <string.h>

#define MAX_DEPTH   8
#define MAX_ENTRIES 64
#define MAX_PATH    256

#define SYM_VERT  "|   "
#define SYM_TEE   "+-- "
#define SYM_LAST  "`-- "
#define SYM_EMPTY "    "

static void print_str(const char *s) {
    write(STDOUT_FILENO, s, strlen(s));
}

static int has_more[MAX_DEPTH + 1];

static void print_prefix(int depth, int is_last) {
    for (int i = 0; i < depth; i++)
        print_str(has_more[i] ? SYM_VERT : SYM_EMPTY);
    print_str(is_last ? SYM_LAST : SYM_TEE);
}
typedef struct {
    unsigned char type;
    char          name[64];
} _snap_t;

static void tree_recurse(const char *path, int depth) {
    if (depth > MAX_DEPTH) return;

    DIR *dir = opendir(path);
    if (!dir) return;

    _snap_t snap[MAX_ENTRIES];
    int total = 0;

    struct dirent *ent;
    while (total < MAX_ENTRIES && (ent = readdir(dir)) != NULL) {
        snap[total].type = ent->d_type;
        // copy only first 63 chars of name (safe for 8.3 + ext paths)
        strncpy(snap[total].name, ent->d_name, 63);
        snap[total].name[63] = '\0';
        total++;
    }

    closedir(dir);

    for (int i = 0; i < total; i++) {
        int is_last = (i == total - 1);
        has_more[depth] = !is_last;
        print_prefix(depth, is_last);

        if (snap[i].type == DT_DIR) {
            write(STDOUT_FILENO, "\033[36m", 5);  // cyan
            print_str(snap[i].name);
            print_str("/");
            write(STDOUT_FILENO, "\033[0m", 4);
            print_str("\n");

            char child[MAX_PATH];
            size_t plen = strlen(path);
            size_t nlen = strlen(snap[i].name);
            if (plen + nlen + 2 <= MAX_PATH) {
                memcpy(child, path, plen);
                if (plen > 0 && child[plen - 1] != '/') child[plen++] = '/';
                memcpy(child + plen, snap[i].name, nlen + 1);
                tree_recurse(child, depth + 1);
            }
        } else {
            print_str(snap[i].name);
            print_str("\n");
        }
    }
}

int main(int argc, char **argv)
{
    const char *path = (argc > 1) ? argv[1] : "/";

    write(STDOUT_FILENO, "\033[36m", 5);
    print_str(path);
    write(STDOUT_FILENO, "\033[0m", 4);
    print_str("\n");

    tree_recurse(path, 0);
    return 0;
}

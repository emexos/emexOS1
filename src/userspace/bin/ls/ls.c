#include <unistd.h>
#include <dirent.h>
#include <string.h>

static void print_str(const char *s) {
    write(STDOUT_FILENO, s, strlen(s));
}

int main(int argc, char **argv)
{
    const char *path = (argc > 1) ? argv[1] : "/";

    DIR *dir = opendir(path);
    if (!dir) {
        print_str("ls: cannot open '");
        print_str(path);
        print_str("'\n");
        return 1;
    }

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (ent->d_type == DT_DIR) {
            write(STDOUT_FILENO, "\033[34m", 5);  // blue for dirs
            print_str(ent->d_name);
            print_str("/");
            write(STDOUT_FILENO, "\033[0m", 4);
        } else {
            print_str(ent->d_name);
        }
        print_str("\n");
    }

    closedir(dir);
    return 0;
}

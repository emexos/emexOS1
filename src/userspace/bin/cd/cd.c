#include <stdio.h>
#include <unistd.h>

//
// NOTE:
// as a standalone binary "cd" only changes the directory of "this" process
// for it to work in a shell it must exec it as a builtin or use the
// result path
// most shells handle cd internally
//

int main(int argc, char *argv[])
{
    if (argc < 2) {
        // no argument: change to root
        if (chdir("/") != 0) {
            puts("cd: failed to change to /");
            return 1;
        }
        return 0;
    }

    if (chdir(argv[1]) != 0) {
        fprintf(stderr, "cd: %s: no such directory\n", argv[1]);
        return 1;
    }

    // print the new cwd so the parent shell can pick it up if needed
    char buf[256];
    if (getcwd(buf, sizeof(buf))) {
        puts(buf);
    }

    return 0;
}
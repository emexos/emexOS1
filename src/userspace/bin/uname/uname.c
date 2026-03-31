#include <stdio.h>
#include <string.h>

#include <emx/sinfo.h>

emx_sinfo_t u;

int main(int argc, char **argv)
{
    if (emx_sinfo(&u) < 0) {
        printf("uname failed\n");
        return 1;
    }

    if (argc == 1) {
        printf("%s\n", u.sysname);
        return 0;
    }

    for (int i = 1; i < argc; i++) {

        if (strcmp(argv[i], "-a") == 0) {
            printf("%s %s %s %s %s\n",
                u.sysname,
                u.nodename,
                u.release,
                u.version,
                u.machine
            );
        }

        else if (strcmp(argv[i], "-s") == 0)
            printf("%s\n", u.sysname);

        else if (strcmp(argv[i], "-n") == 0)
            printf("%s\n", u.nodename);

        else if (strcmp(argv[i], "-r") == 0)
            printf("%s\n", u.release);

        else if (strcmp(argv[i], "-v") == 0)
            printf("%s\n", u.version);

        else if (strcmp(argv[i], "-m") == 0)
            printf("%s\n", u.machine);
    }

    return 0;
}
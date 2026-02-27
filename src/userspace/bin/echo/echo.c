#include <unistd.h>
#include <string.h>

int main(int argc, char **argv)
{
    // no args, just print newline
    if (argc < 2) {
        write(STDOUT_FILENO, "\n", 1);
        return 0;
    }

    // print each arg separated by space
    for (int i = 1; i < argc; i++) {
        size_t len = strlen(argv[i]);
        write(STDOUT_FILENO, argv[i], len);

        if (i < argc - 1)
            write(STDOUT_FILENO, " ", 1);
    }

    write(STDOUT_FILENO, "\n", 1);
    return 0;
}

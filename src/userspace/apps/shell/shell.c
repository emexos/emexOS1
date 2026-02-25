#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define BUFFER 256

#define SHELL_PROMPT "[pc@emexos]$"

int main(void)
{
    char buf[BUFFER];

    for (;;)
    {
        write(STDOUT_FILENO, SHELL_PROMPT " ", sizeof(SHELL_PROMPT " ") - 1);

        // block until newline (\n)
        ssize_t n = read(STDIN_FILENO, buf, sizeof(buf) - 1);

        if (n <= 0) continue;

        buf[n] = '\0';

        if (n > 0 && buf[n - 1] == '\n') buf[--n] = '\0';
        if (n == 0) continue;

        // TODO: command parser
    }

    return 0;
}

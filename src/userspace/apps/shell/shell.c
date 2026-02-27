#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define BUFFER 256
#define SHELL_PROMPT "[pc@emexos]$ "
#define BIN_PATH "/bin/"


static int parse_args(char *buf, char **argv, int max_args)
{
    int argc = 0;
    char *p = buf;

    while (*p && argc < max_args - 1) {
        // skip spaces
        while (*p == ' ') p++;
        if (*p == '\0') break;

        argv[argc++] = p;

        // find end of token
        while (*p && *p != ' ') p++;
        if (*p == ' ') *p++ = '\0';
    }

    argv[argc] = NULL;
    return argc;
}
static int exec_from_bin(const char *cmd, char **argv)
{
    // /bin/ + cmd + .elf
    char path[BUFFER];
    char *const envp[] = { (char *)0 };
    size_t bin_len = sizeof(BIN_PATH) - 1; // length of "/bin/"
    size_t cmd_len = strlen(cmd);
    size_t elf_len = 4; // ".elf"

    if (bin_len + cmd_len + elf_len + 1 > BUFFER) return -1;

    memcpy(path, BIN_PATH, bin_len);
    memcpy(path + bin_len, cmd, cmd_len);
    memcpy(path + bin_len + cmd_len, ".elf", elf_len + 1); // +1 for \0

    return execve(path, argv, envp);
}

int main(void)
{
    char buf[BUFFER];
    char *argv[32];

    for (;;)
    {
        write(STDOUT_FILENO, SHELL_PROMPT, sizeof(SHELL_PROMPT) - 1);

        // block until newline (\n)
        ssize_t n = read(STDIN_FILENO, buf, sizeof(buf) - 1);

        if (n <= 0) continue;

        buf[n] = '\0';

        if (n > 0 && buf[n - 1] == '\n') buf[--n] = '\0';
        if (n == 0) continue;

        int argc = parse_args(buf, argv, 32);
        if (argc == 0) continue;

        int ret = exec_from_bin(argv[0], argv);
        if (ret < 0) {
            write(STDOUT_FILENO, argv[0], strlen(argv[0]));
            write(STDOUT_FILENO, ": command not found\n", 20);
        }
    }

    return 0;
}

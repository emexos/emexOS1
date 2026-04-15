#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>


//-////////////////////////////////////////-//
//-//                                    //-//
//-//       emex user-space shell        //-//
//-//                                    //-//
//-////////////////////////////////////////-//

#define BUFFER 256
#define SHELL_PROMPT "\n\033[0mx1:\033[31m[pc@emex]#\033[0m "
#define SHELL_CONFIG "/.config/exsh/"
#define BIN_PATH "/bin/"

#define WELCOME_MESSAGE "\n\n\033[0m Welcome to eXsh, emexOS's default shell.\n Type \"ls /bin\" for a list of commands.\n\n"


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

static void builtin_exec(char **argv)
{
    if (!argv[1]) {
        printf("exec: missing path\n");
        return;
    }

    const char *path = argv[1];
    char *const envp[] = { (char *)0 };

    // check if it ends with .emx
    size_t len = strlen(path);
    int is_emx = (len > 4 && strcmp(path + len - 4, ".emx") == 0);

    pid_t pid = fork();
    if (pid == 0) {
        if (is_emx) {
            execve(path, argv + 1, envp);
        } else {
            execve(path, argv + 1, envp);
        }
        printf("exec: failed to run: ");
        printf(path);
        printf("\n");
        _exit(1);
    } else if (pid > 0) {
        waitpid(pid, NULL, 0);
    } else {
        printf("exec: fork failed\n");
    }
}

int main(void)
{
	printf(WELCOME_MESSAGE);

    char buf[BUFFER];
    char *argv[32];

    for (;;)
    {
        printf(SHELL_PROMPT);

        // block until newline (\n)
        ssize_t n = read(STDIN_FILENO, buf, sizeof(buf) - 1);

        if (n <= 0) continue;

        buf[n] = '\0';

        if (n > 0 && buf[n - 1] == '\n') buf[--n] = '\0';
        if (n == 0) continue;

        int argc = parse_args(buf, argv, 32);
        if (argc == 0) continue;

        builtins: {
	        if (strcmp(argv[0], "exec") == 0) {
	            builtin_exec(argv);
	            continue;
	        }
        }

        int ret = exec_from_bin(argv[0], argv);
        if (ret < 0) {
            printf(argv[0]);
            printf(": command not found\n");
        }
    }

    return 0;
}

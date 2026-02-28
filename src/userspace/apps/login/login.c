#include <unistd.h>
#include <string.h>

#include "login.h"

//TODO:
// implement password hashing

static void print_str(const char *s) {
    size_t len = strlen(s);
    write(STDOUT_FILENO, s, len);
}
static int str_eq(const char *a, const char *b) {
    while (*a && *b) {
        if (*a != *b) return 0;
        a++; b++;
    }
    return (*a == '\0' && *b == '\0');
}
static void strip_nl(char *buf, int len) {
    for (int i = 0; i < len; i++) {
        if (buf[i] == '\n' || buf[i] == '\r') {
            buf[i] = '\0';
            return;
        }
    }
}

int main(void)
{
    char user_buf[BUF_SIZE];
    char pass_buf[BUF_SIZE];

    print_str("\n");
    print_str("----------------------\n");
    print_str(" [emexOS login]\n");

    for (int attempt = 0; attempt < MAX_TRIES; attempt++)
    {
        print_str("\033[34m login: ");
        ssize_t n = read(STDIN_FILENO, user_buf, BUF_SIZE - 1);
        if (n <= 0) continue;
        user_buf[n] = '\0';
        strip_nl(user_buf, (int)n);

        print_str("\033[34m password: ");
        ssize_t m = read(STDIN_FILENO, pass_buf, BUF_SIZE - 1);
        if (m <= 0) continue;
        pass_buf[m] = '\0';
        strip_nl(pass_buf, (int)m);

        if (str_eq(user_buf, LOGIN_USER) && str_eq(pass_buf, LOGIN_PASS))
        {
            print_str("\033[0m----------------------\n");
            print_str("\n\033[36mwelcome, ");
            print_str(user_buf);
            print_str("!\n");
            print_str("\033[0m");

            // launch the shell
            //char *const argv[] = { (char *)SHELL_PATH, (char *)0 };
            //char *const envp[] = { (char *)0 };
            //execve(SHELL_PATH, argv, envp);

            //should not return
            //print_str("error: failed to launch shell\n");
            return 0;
        }

        print_str("\033[0m[login incorrect]\n");
    }
    print_str("\033[0m----------------------\n");
    print_str("\033[0m\ntoo many failed attempts\n");

    return 2;
}

#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

#include "login.h"

#include <emx/ansi.h>

#define LOGO_BIG 1
#define LOGO_SMALL 0

//TODO:
// implement password hashing

/*
static void print_str(const char *s) {
    write(STDOUT_FILENO, s, strlen(s));
}*/

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

    printf("\n");

    print_logo: {
    	#if LOGO_SMALL == 1
    	#	define LOGO_PATH "/emr/assets/logo_small.txt"
     	#elif LOGO_BIG == 1
      	#	define LOGO_PATH "/emr/assets/logo.txt"
      	#endif

     	#define LOGO_BUF 2048

        int fd = open(LOGO_PATH, O_RDONLY);
	    if (fd < 0) {
	      	goto login;
	    } else {
	        char buf[LOGO_BUF];
	        ssize_t n = read(fd, buf, LOGO_BUF - 1);
	        close(fd);
	        if (n > 0) {
	            buf[n] = '\0';
	            printf("\033[96m");
	            write(STDOUT_FILENO, buf, (size_t)n);
	            printf("\033[0m\n");
	        }
			goto login;
	    }
    }

    printf("if this is reached this is bad :(");

login:
    printf("\n\n");
    //printf("----------------------\n");
    printf(" [emexOS login]\n");

    for (int attempt = 0; attempt < MAX_TRIES; attempt++)
    {
        printf(A_GFX_YELLOW " login:\033[0m ");
        ssize_t n = read(STDIN_FILENO, user_buf, BUF_SIZE - 1);
        if (n <= 0) continue;
        user_buf[n] = '\0';
        strip_nl(user_buf, (int)n);

        printf(A_GFX_YELLOW " password:\033[0m ");
        ssize_t m = read(STDIN_FILENO, pass_buf, BUF_SIZE - 1);
        if (m <= 0) continue;
        pass_buf[m] = '\0';
        strip_nl(pass_buf, (int)m);

        if (str_eq(user_buf, LOGIN_USER) && str_eq(pass_buf, LOGIN_PASS))
        {
            //printf("\033[0m----------------------\n");
            //printf("\n");
            printf("\n \033[31m>\033[32m>\033[35m>\033[36m welcome, ");
            printf(user_buf);
            printf("!\n\n\033[0m");

            // launch the shell
            //char *const argv[] = { (char *)SHELL_PATH, (char *)0 };
            //char *const envp[] = { (char *)0 };
            //execve(SHELL_PATH, argv, envp);

            //should not return
            //print_str("error: failed to launch shell\n");
            return 0;
        }

        printf("\033[31m [login incorrect]\033[0m\n");
    }
    //printf("\033[0m----------------------\n");
    printf("\033[0m\n too many failed attempts\n");

    return 2;
}

#pragma once
#include <sys/types.h>
#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2


// i/o
ssize_t read(int fd, void *buf, size_t n);
ssize_t write(int fd, const void *buf, size_t n);

// filesystem
int chdir (const char *path);
int mkdir (const char *path);
char *getcwd (char *buf, size_t size);
int unlink(const char *path);

// process
pid_t getpid (void);
pid_t waitpid(pid_t pid, int *status, int options);
pid_t fork (void);
void _exit (int status) __attribute__((noreturn));
int execve (const char *path, char *const argv[], char *const envp[]);

static inline pid_t spawn(const char *path) {
    pid_t pid = fork();
    if (pid == 0) {
        // spawn child
        char *const argv[] = { (char *)path, (char *)0 };
        char *const envp[] = { (char *)0 };
        execve(path, argv, envp);
        _exit(1); // execve failed
    }
    return pid; // parent gets pid immediately
    // no wait
}
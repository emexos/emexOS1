#include <unistd.h>
#include "_syscall.h"

// no errno access

ssize_t read(int fd, void *buf, size_t n) {
    return (ssize_t)_sc3(_SCAL_READ, fd, (long)buf, (long)n);
}

ssize_t write(int fd, const void *buf, size_t n) {
    return (ssize_t)_sc3(_SCAL_WRITE, fd, (long)buf, (long)n);
}

pid_t getpid(void) {
    return (pid_t)_sc1(_SCAL_GETPID, 0);
}

pid_t fork(void) {
    return (pid_t)_sc1(_SCAL_FORK, 0);
}

void _exit(int status) {
    _sc1(_SCAL_EXIT, status);
    __builtin_unreachable();
}

int execve(const char *path, char *const argv[], char *const envp[]) {
    (void)argv; (void)envp;
    return (int)_sc1(_SCAL_EXECVE, (long)path);
}

int chdir(const char *path) {
    return (int)_sc1(_SCAL_CHDIR, (long)path);
}

int mkdir(const char *path) {
    return (int)_sc1(_SCAL_MKDIR, (long)path);
}

char *getcwd(char *buf, size_t size) {
    long r = _sc2(_SCAL_GETCWD, (long)buf, (long)size);
    return (r > 0) ? buf : NULL;
}

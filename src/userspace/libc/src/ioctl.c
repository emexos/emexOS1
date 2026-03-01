#include <sys/ioctl.h>
#include "_syscall.h"

//
//#define SYS_IOCTL 16

int ioctl(int fd, int request, void *arg) {
    long ret;
    __asm__ volatile (
        "syscall"
        : "=a"(ret)
        : "0"(_SCAL_IOCTL), "D"((long)fd), "S"((long)request), "d"((long)arg)
        : "rcx", "r11", "memory"
    );
    return (int)ret;
}
#pragma once

#define _SCAL_READ    0
#define _SCAL_WRITE   1
#define _SCAL_OPEN    2
#define _SCAL_CLOSE   3
#define _SCAL_BRK     12
#define _SCAL_GETPID  39
#define _SCAL_FORK    57
#define _SCAL_EXECVE  59
#define _SCAL_EXIT    60
#define _SCAL_GETCWD  79
#define _SCAL_CHDIR   80
#define _SCAL_MKDIR   83
#define _SCAL_UNLINK  87  // (delete file)


static inline long _sc1(long n, long a1) {
    long r;
    __asm__ volatile("syscall":"=a"(r):"a"(n),"D"(a1):"rcx","r11","memory");
    return r;
}
static inline long _sc2(long n, long a1, long a2) {
    long r;
    __asm__ volatile("syscall":"=a"(r):"a"(n),"D"(a1),"S"(a2):"rcx","r11","memory");
    return r;
}
static inline long _sc3(long n, long a1, long a2, long a3) {
    long r;
    __asm__ volatile("syscall":"=a"(r):"a"(n),"D"(a1),"S"(a2),"d"(a3):"rcx","r11","memory");
    return r;
}

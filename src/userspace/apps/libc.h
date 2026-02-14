#pragma once

typedef unsigned long size_t;
typedef long ssize_t;

static inline size_t syscall3(size_t n, size_t a1, size_t a2, size_t a3) {
    size_t ret;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2), "d"(a3) : "rcx", "r11", "memory");
    return ret;
}

static inline size_t syscall1(size_t n, size_t a1) {
    size_t ret;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(n), "D"(a1) : "rcx", "r11", "memory");
    return ret;
}

static inline ssize_t write(int fd, const void *buf, size_t count) {
    return (ssize_t)syscall3(1, (size_t)fd, (size_t)buf, (size_t)count);
}

static inline void exit(int status) {
    syscall1(60, (size_t)status);
    __builtin_unreachable();
}

static inline size_t strlen(const char *s) {
    size_t len = 0;
    while (s[len]) len++;
    return len;
}

static inline void print(const char *s) {
    write(1, s, strlen(s));
}

#pragma once
#include <sys/types.h>

// open flags (needs to vfs.h)
#define O_RDONLY 0x01
#define O_WRONLY 0x02
#define O_RDWR 0x03
#define O_CREAT 0x04

int open(const char *path, int flags, ...);
int close(int fd);

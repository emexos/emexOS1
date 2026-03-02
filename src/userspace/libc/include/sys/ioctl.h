#pragma once
#include <sys/types.h>
#include <emx/fb.h>
#include <emx/tty.h>

// return:
//  0 == success
// -1 == error
int ioctl(int fd, int request, void *arg);
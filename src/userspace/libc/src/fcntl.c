#include <fcntl.h>
#include <errno.h>
#include "_syscall.h"

int open(const char *path, int flags, ...)
{
    long r = _sc2(_SCAL_OPEN, (long)path, (long)flags);
    if (r < 0) { errno = (int)-r; return -1; }
    return (int)r;
}

int close(int fd)
{
    long r = _sc1(_SCAL_CLOSE, fd);
    if (r < 0) { errno = (int)-r; return -1; }
    return 0;
}

#include <unistd.h>
#include "_syscall.h"

pid_t waitpid(pid_t pid, int *status, int options) {
    (void)status; (void)options;
    return (pid_t)_sc1(_SCAL_WAITPID, (long)pid);
}

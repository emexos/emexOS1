#pragma once
#include <sys/types.h>

pid_t waitpid(pid_t pid, int *status, int options);
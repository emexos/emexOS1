#include <emx/system.h>
#include "_syscall.h"

int reboot(unsigned long cmd) {
    long r;
    __asm__ volatile(
        "syscall"
        : "=a"(r)
        : "a"((long)_SCAL_REBOOT),
          "D"((long)RSYSTEM_MAGIC1),
          "S"((long)RSYSTEM_MAGIC2),
          "d"((long)cmd)
        : "rcx", "r11", "memory"
    );
    return (int)r;
}

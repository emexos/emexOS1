#include <types.h>
#include <kernel/include/ports.h>
#include "../console.h"

/* -------------------------------------------------------------------------------------------------------
 *
 * from https: //codeberg.org/warposteam/warpos-src/src/branch/warp/source/monolithic/shell/poweroff.zig#
 * converted to c
 *
 * -------------------------------------------------------------------------------------------------------
 */

/// x86_64 specific poweroff implementation using QEMU's shutdown mechanism
/// Writes to the QEMU-specific I/O port to trigger system shutdown

#define POWEROFF_SHUTDOWN 0
#define POWEROFF_REBOOT   1

static inline void x86_poweroff(void) {
    __asm__ volatile (
        "outw %0, %1"
        :
        : "a" ((u16)0x2000),
        "d" ((u16)0x604)
        : "memory"
    );
}

static inline void x86_restart(void) {
    __asm__ volatile (
        "movb $0xFE, %%al\n"
        "outb %%al, $0x64\n"
        :
        :
        : "al"
    );
    __asm__ volatile (
        "movw $0xCF9, %%dx\n"
        "inb %%dx, %%al\n"
        "orb $0x6, %%al\n"
        "outb %%al, %%dx\n"
        :
        :
        : "al", "dx"
    );
    __asm__ volatile (
        "lidt 0\n"
        "int $3\n"
    );
}

static inline void x86_shutdown(void) {
    __asm__ volatile (
        "movw $0x604, %%dx\n"
        "movw $0x2000, %%ax\n"
        "outw %%ax, %%dx\n"
        :
        :
        : "ax", "dx"
    );
    __asm__ volatile (
        "movw $0x5307, %%ax\n"
        "movw $0x0001, %%bx\n"
        "movw $0x0003, %%cx\n"
        "int $0x15\n"
        :
        :
        : "ax", "bx", "cx"
    );
    x86_poweroff();
}

__attribute__((visibility("default")))
int poweroff(int operation) {
#if defined(__x86_64__)
    if (operation == POWEROFF_REBOOT) {
        #ifdef QEMU_BUILD
            x86_poweroff();
        #else
            x86_restart();
        #endif
    } else if (operation == POWEROFF_SHUTDOWN) {
        #ifdef QEMU_BUILD
            x86_poweroff();
        #else
            x86_shutdown();
        #endif
    } else {
        return -1;
    }
    return 0;
#else
    return -1;
#endif
}
FHDR(cmd_poweroff) {
    (void)s;
    print("Shutting down system...\n", GFX_YELLOW);
    print("Note: May not work on all hardware configurations\n", GFX_RED);
    int result = poweroff(POWEROFF_SHUTDOWN);
    if (result == -1) {
        print("Shutdown failed\n", GFX_RED);
    }
}

FHDR(cmd_reboot) {
    (void)s;
    print("Restarting system...\n", GFX_YELLOW);
    print("Note: May not work on all hardware configurations\n", GFX_RED);
    int result = poweroff(POWEROFF_REBOOT);
    if (result == -1) {
        print("Restart failed\n", GFX_RED);
    }
}

FHDR(cmd_shutdown) {
    (void)s;
    print("Shutting down system...\n", GFX_YELLOW);
    print("Note: May not work on all hardware configurations\n", GFX_RED);
    int result = poweroff(POWEROFF_SHUTDOWN);
    if (result == -1) {
        print("Shutdown failed\n", GFX_RED);
    }
}

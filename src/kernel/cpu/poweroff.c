#include "poweroff.h"

#include <types.h>
#include <kernel/include/ports.h>

//#define POWEROFF_SHUTDOWN 0
//#define POWEROFF_REBOOT   1

static inline void x86_restart(void)
{
    // Keyboard Controller Reset (most reliable)
    outb(0x64, 0xFE);

    for (volatile int i = 0; i < 1000000; i++)
        __asm__ volatile("nop");

    // PCI RCR
    u8 temp = inb(0xCF9);
    outb(0xCF9, temp | 0x02);
    outb(0xCF9, temp | 0x06);

    for (volatile int i = 0; i < 1000000; i++)
        __asm__ volatile("nop");
}

static inline void x86_shutdown(void)
{
    // QEMU / Bochs
    outw(0x604, 0x2000);

    for (volatile int i = 0; i < 10000000; i++)
        __asm__ volatile("nop");

    // Fallbacks
    outw(0xB004, 0x2000);
    outw(0x0604, 0x2000);

    for (volatile int i = 0; i < 10000000; i++)
        __asm__ volatile("nop");
}

__attribute__((visibility("default")))
void cpu_poweroff(int operation)
{
#if defined(__x86_64__)
    __asm__ volatile("cli" ::: "memory");

    if (operation == POWEROFF_REBOOT)
        x86_restart();
    else if (operation == POWEROFF_SHUTDOWN)
        x86_shutdown();

    while (1)
        __asm__ volatile("hlt" ::: "memory");
#else
    (void)operation;
#endif
}

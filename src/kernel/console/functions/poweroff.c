#include <kernel/cpu/poweroff.h>
#include <kernel/console/console.h>

FHDR(cmd_reboot)
{
    (void)s;

    cursor_disable();
    print("Restarting system...\n", GFX_YELLOW);

    for (volatile int i = 0; i < 5000000; i++)
        __asm__ volatile("nop");

    cpu_poweroff(POWEROFF_REBOOT);
}

FHDR(cmd_shutdown)
{
    (void)s;

    cursor_disable();
    print("Shutting down system...\n", GFX_YELLOW);

    for (volatile int i = 0; i < 5000000; i++)
        __asm__ volatile("nop");

    cpu_poweroff(POWEROFF_SHUTDOWN);
}

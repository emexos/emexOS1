#include <types.h>
#include <kernel/include/ports.h>
#include "../console.h"

/* -------------------------------------------------------------------------------------------------------
 * Simplified poweroff implementation
 * FIXED: Removed all INT instructions that cause Reserved exceptions
 * -------------------------------------------------------------------------------------------------------
 */

#define POWEROFF_SHUTDOWN 0
#define POWEROFF_REBOOT   1

static inline void x86_poweroff(void) {
    // QEMU/Bochs magic shutdown port
    outw(0x604, 0x2000);
}

static inline void x86_restart(void) {
    // Method 1: Keyboard Controller Reset (most reliable)
    outb(0x64, 0xFE);

    // Small delay
    for (volatile int i = 0; i < 1000000; i++) {
        __asm__ volatile("nop");
    }

    // Method 2: PCI Reset Control Register
    u8 temp = inb(0xCF9);
    outb(0xCF9, temp | 0x02);  // Request reset
    outb(0xCF9, temp | 0x06);  // Actually reset

    // Small delay
    for (volatile int i = 0; i < 1000000; i++) {
        __asm__ volatile("nop");
    }
}

static inline void x86_shutdown(void) {
    // Try QEMU/Bochs shutdown port
    outw(0x604, 0x2000);

    // Small delay to let it work
    for (volatile int i = 0; i < 10000000; i++) {
        __asm__ volatile("nop");
    }

    // If that didn't work, try ACPI shutdown (simple version)
    // This is for real hardware - won't work in QEMU but won't crash
    outw(0xB004, 0x2000);  // Bochs/Old QEMU
    outw(0x0604, 0x2000);  // QEMU alternative

    // One more delay
    for (volatile int i = 0; i < 10000000; i++) {
        __asm__ volatile("nop");
    }
}

__attribute__((visibility("default")))
int poweroff(int operation) {
#if defined(__x86_64__)
    // Disable interrupts to prevent issues during shutdown
    __asm__ volatile("cli" ::: "memory");

    if (operation == POWEROFF_REBOOT) {
        x86_restart();
    } else if (operation == POWEROFF_SHUTDOWN) {
        x86_shutdown();
    } else {
        return -1;
    }

    // If we get here, shutdown/reboot didn't work
    // Just halt the CPU
    while(1) {
        __asm__ volatile("hlt" ::: "memory");
    }

    return 0;
#else
    (void)operation;
    return -1;
#endif
}

FHDR(cmd_poweroff) {
    (void)s;

    cursor_disable();  // Disable cursor before shutdown
    print("Shutting down system...\n", GFX_YELLOW);

    // Give time for message to display
    for (volatile int i = 0; i < 5000000; i++) {
        __asm__ volatile("nop");
    }

    poweroff(POWEROFF_SHUTDOWN);

    // Should not reach here, but just in case
    while(1) {
        __asm__ volatile("cli; hlt" ::: "memory");
    }
}

FHDR(cmd_reboot) {
    (void)s;

    cursor_disable();  // Disable cursor before reboot
    print("Restarting system...\n", GFX_YELLOW);

    // Give time for message to display
    for (volatile int i = 0; i < 5000000; i++) {
        __asm__ volatile("nop");
    }

    poweroff(POWEROFF_REBOOT);

    // Should not reach here, but just in case
    while(1) {
        __asm__ volatile("cli; hlt" ::: "memory");
    }
}

FHDR(cmd_shutdown) {
    (void)s;

    cursor_disable();  // Disable cursor before shutdown
    print("Shutting down system...\n", GFX_YELLOW);

    // Give time for message to display
    for (volatile int i = 0; i < 5000000; i++) {
        __asm__ volatile("nop");
    }

    poweroff(POWEROFF_SHUTDOWN);

    // Should not reach here, but just in case
    while(1) {
        __asm__ volatile("cli; hlt" ::: "memory");
    }
}

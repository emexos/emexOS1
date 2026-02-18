#include <kernel/console/console.h>
#include <kernel/kernelslot/slot.h>
//#include <kernel/cpu/poweroff.h>

// ----
// NOTE:
// Switching the active boot slot is a low-level operation,
// this directly modifies the boot configuration used by
// the system!
// ----

// ---
// this is a test function it will help us wenn updating the kernel
// ---

FHDR(cmd_slot)
{
    if (!s || str_len(s) == 0) {
        char current = readslot();

        char msg[32] = "current active slot: ";
        char slot_str[2];
        slot_str[0] = current;
        slot_str[1] = 0;

        str_append(msg, slot_str);
        str_append(msg, "\n");

        cprintf(msg, GFX_GREEN);
        return;
    }

    if (str_starts_with(s, "set ")) {

        char slot = s[4];

        if (slot != 'A' && slot != 'B' &&
            slot != 'a' && slot != 'b') {
            cprintf("wrong slot num, use \"A\" or \"B\"\n", GFX_RED);
            return;
        }

        cprintf("this does currently not switch the real kernel slot!\n", GFX_YELLOW);

        if (slot == 'a') slot = 'A';
        if (slot == 'b') slot = 'B';

        char current = readslot();

        if (current == slot) {
            cprintf("slot is already active\n", GFX_YELLOW);
            return;
        }

        if (writeslot(slot) == 0) {
            cprintf("switch slot and reboot...\n", GFX_YELLOW);

            for (volatile int i = 0; i < 5000000; i++)
                __asm__ volatile("nop");

            cpu_poweroff(POWEROFF_REBOOT);
        } else {
            cprintf("failed to switch slot\n", GFX_RED);
        }

        return;
    }
    return;
}

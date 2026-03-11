#include "panic.h"

#include <kernel/graph/graphics.h>
#include <kernel/graph/theme.h>
#include <theme/doccr.h>
#include <kernel/include/ports.h>
#include <kernel/include/assembly.h>
#include <kernel/kernel_processes/fm/fm.h>

//#define BOOTUP_VISUALS 0

#define PANIC_BG PANICSCREEN_BG_COLOR
#define PANIC_FG PANICSCREEN_FG_COLOR

// from the old console poweroff
static void panic_reboot(void) {
    outb(0x64, 0xFE);
    for (volatile int i = 0; i < 1000000; i++) __asm__ volatile("nop");

    u8 tmp = inb(0xCF9);
    outb(0xCF9, tmp | 0x02);
    outb(0xCF9, tmp | 0x06);
    for (volatile int i = 0; i < 1000000; i++) __asm__ volatile("nop");

    // nothing worked, halt forever
    log("\n::", "SHUTDOWN FAILED, HALTING FOREVER.", _d);
    while (1) __asm__ volatile("cli; hlt");
}

__attribute__((noreturn)) void panic(const char *message)
{
	log("\n::", "PANIC WILL BE EXECUTED IN 100000000 TICKS\n\n", _d);
	delay(10);

    setcontext(THEME_PANIC);
    clear(PANICSCREEN_BG_COLOR);
    f_setcontext(PANIC_FONT);
    // disable interrupts
    __asm__ volatile("cli");

    print("\n", PANIC_FG);
    print("--- KERNEL PANIC --- ", PANIC_FG);
    print("\n", PANIC_FG);

    f_setcontext(FONT_8X8);

    print("\nIt seems that emexOS has encountered an error.\n\n", PANIC_FG);

    if (message) {
        print(message, PANIC_FG);
        print("\n", PANIC_FG);
    }

    print("System halted, \nYour computer will now restart...", PANIC_FG);

    delay(50);
    panic_reboot();

    // HALT
    //while(1) {
    //    __asm__ volatile("cli; hlt");
    //}
}

__attribute__((noreturn)) void panic_exception(cpu_state_t *state, const char *message)
{
 	u64 cr2, cr3;
    __asm__ volatile("mov %%cr2, %0" : "=r"(cr2));
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));

    log("\n::", "PANIC WILL BE EXECUTED IN 100000000 TICKS\n\n", _d);
	delay(10);

    setcontext(THEME_PANIC);
    clear(PANICSCREEN_BG_COLOR);
    f_setcontext(PANIC_FONT);
    // Disable interrupts
    __asm__ volatile("cli");

    print("\n", PANIC_FG);
    print("--- KERNEL PANIC --- ", PANIC_FG);
    print("\n", PANIC_FG);

    f_setcontext(FONT_8X8);

    print("\nIt seems that emexOS has encountered an error.\n\n", PANIC_FG);

    if (message) {
        char buf[128];
        str_copy(buf, "Exception: ");
        str_append(buf, message);
        print(buf, PANIC_FG);
        print("\n", PANIC_FG);
    }

    // Print exception details
    char buf[128];
    str_copy(buf, "INT: ");
    str_append_uint(buf, (u32)state->int_no);
    str_append(buf, " ERR: ");
    str_append_uint(buf, (u32)state->err_code);
    print(buf, PANIC_FG);
    print("\n", PANIC_FG);

    // Print RIP
    char hex[32];
    str_copy(buf, "RIP: 0x");
    str_from_hex(hex, state->rip);
    str_append(buf, hex);
    print(buf, PANIC_FG);
    print("\n", PANIC_FG);

    // Print RSP
    str_copy(buf, "RSP: 0x");
    str_from_hex(hex, state->rsp);
    str_append(buf, hex);
    print(buf, PANIC_FG);
    print("\n\n", PANIC_FG);

    // Print RFLAGS
    str_copy(buf, "RFLAGS: 0x");
    str_from_hex(hex, state->rflags);
    str_append(buf, hex);
    print(buf, white());
    print("\n", white());

    if (state->int_no == 14) {
        str_copy(buf, "CR2: 0x");
        str_from_hex(hex, cr2);
        str_append(buf, hex);
        str_append(buf, "  (faulting address)");
        print(buf, PANIC_FG);
        print("\n", white());

        str_copy(buf, "Fault: ");
        if (state->err_code & 1)
        	 str_append(buf,  "prot-violation ");
        else str_append(buf,  "not-present ");
        if (state->err_code & 2)
        	 str_append(buf,  "write ");
        else str_append(buf,  "read ");
        if (state->err_code & 4)
        	 str_append(buf,  "user-mode ");
        else str_append(buf,  "kernel-mode ");
        if (state->err_code & 8)
        	 str_append(buf,  "reserved-bits ");
        if (state->err_code & 16)
        	 str_append(buf,  "instruction-fetch ");
        print(buf, PANIC_FG);
        print("\n", white());
    }

    str_copy(buf, "CR3: 0x");
    str_from_hex(hex, cr3);
    str_append(buf, hex);
    print(buf, white());
    print("\n\n", white());

    print("System halted, \nYour computer will now restart...", PANIC_FG);

    delay(50);
    panic_reboot();

    // HALT
    //while(1) {
    //    __asm__ volatile("cli; hlt");
    //}
}

#include "panic.h"

#include <kernel/graph/graphics.h>
#include <kernel/graph/theme.h>
#include <theme/doccr.h>
#include <kernel/kernel_processes/fm/fm.h>

//#define BOOTUP_VISUALS 0

#define PANIC_BG PANICSCREEN_BG_COLOR
#define PANIC_FG PANICSCREEN_FG_COLOR

__attribute__((noreturn)) void panic(const char *message)
{
	print("\nPANIC WILL BE EXECUTED IN 100000000 TICKS\n\n", white());
 	for (volatile int i = 0; i < 100000000; i++) {
        nop();
    }
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

    // HALT
    while(1) {
        __asm__ volatile("cli; hlt");
    }
}

__attribute__((noreturn)) void panic_exception(cpu_state_t *state, const char *message)
{
	print("\nPANIC WILL BE EXECUTED IN 100000000 TICKS\n\n", white());
	for (volatile int i = 0; i < 100000000; i++) {
        nop();
    }
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

    print("System halted, \nYour computer will now restart...", PANIC_FG);

    // HALT
    while(1) {
        __asm__ volatile("cli; hlt");
    }
}

#include "panic.h"
#include <kernel/graph/graphics.h>
#include <theme/doccr.h>
#include <kernel/graph/theme.h>
#include <theme/doccr.h>

__attribute__((noreturn)) void panic(const char *message)
{
    setcontext(THEME_PANIC);
    clear(PANICSCREEN_BG_COLOR);
    // Disable interrupts
    __asm__ volatile("cli");

    print("\n", white());
    print("!!! --- KERNEL PANIC --- !!!", red());
    print("\n", white());

    if (message) {
        print(message, red());
        print("\n", white());
    }

    print("\nSystem halted.", white());

    // HALT
    while(1) {
        __asm__ volatile("cli; hlt");
    }
}

__attribute__((noreturn)) void panic_exception(cpu_state_t *state, const char *message)
{
    setcontext(THEME_PANIC);
    clear(PANICSCREEN_BG_COLOR);
    // Disable interrupts
    __asm__ volatile("cli");

    print("\n", white());
    print("!!! PANIC !!!", red());
    print("\n", white());

    if (message) {
        char buf[128];
        str_copy(buf, "Exception: ");
        str_append(buf, message);
        print(buf, red());
        print("\n", white());
    }

    // Print exception details
    char buf[128];
    str_copy(buf, "INT: ");
    str_append_uint(buf, (u32)state->int_no);
    str_append(buf, " ERR: ");
    str_append_uint(buf, (u32)state->err_code);
    print(buf, white());
    print("\n", white());

    // Print RIP
    char hex[32];
    str_copy(buf, "RIP: 0x");
    str_from_hex(hex, state->rip);
    str_append(buf, hex);
    print(buf, white());
    print("\n", white());

    // Print RSP
    str_copy(buf, "RSP: 0x");
    str_from_hex(hex, state->rsp);
    str_append(buf, hex);
    print(buf, white());
    print("\n\n", white());

    print("System halted.", white());

    // HALT
    while(1) {
        __asm__ volatile("cli; hlt");
    }
}

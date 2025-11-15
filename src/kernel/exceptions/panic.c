#include "panic.h"
#include <klib/graphics/graphics.h>
#include <theme/doccr.h>

__attribute__((noreturn)) void panic(const char *message)
{
    clear(PANICSCREEN_BG_COLOR);
    // Disable interrupts
    __asm__ volatile("cli");

    print("\n", PANICSCREEN_COLOR);
    print("!!! --- KERNEL PANIC --- !!!", PANICSCREEN_COLOR_R);
    print("\n", PANICSCREEN_COLOR);

    if (message) {
        print(message, PANICSCREEN_COLOR_R);
        print("\n", PANICSCREEN_COLOR);
    }

    print("\nSystem halted.", PANICSCREEN_COLOR);

    // HALT
    while(1) {
        __asm__ volatile("cli; hlt");
    }
}

__attribute__((noreturn)) void panic_exception(cpu_state_t *state, const char *message)
{
    clear(PANICSCREEN_BG_COLOR);
    // Disable interrupts
    __asm__ volatile("cli");

    print("\n", PANICSCREEN_COLOR);
    print("!!! PANIC !!!", PANICSCREEN_COLOR_R);
    print("\n", PANICSCREEN_COLOR);

    if (message) {
        char buf[128];
        str_copy(buf, "Exception: ");
        str_append(buf, message);
        print(buf, PANICSCREEN_COLOR_R);
        print("\n", PANICSCREEN_COLOR);
    }

    // Print exception details
    char buf[128];
    str_copy(buf, "INT: ");
    str_append_uint(buf, (u32)state->int_no);
    str_append(buf, " ERR: ");
    str_append_uint(buf, (u32)state->err_code);
    print(buf, PANICSCREEN_COLOR);
    print("\n", PANICSCREEN_COLOR);

    // Print RIP
    str_copy(buf, "RIP: 0x");
    str_append_uint(buf, (u32)(state->rip >> 32));
    str_append_uint(buf, (u32)(state->rip & 0xFFFFFFFF));
    print(buf, PANICSCREEN_COLOR);
    print("\n", PANICSCREEN_COLOR);

    // Print RSP
    str_copy(buf, "RSP: 0x");
    str_append_uint(buf, (u32)(state->rsp >> 32));
    str_append_uint(buf, (u32)(state->rsp & 0xFFFFFFFF));
    print(buf, PANICSCREEN_COLOR);
    print("\n\n", PANICSCREEN_COLOR);

    print("System halted.", PANICSCREEN_COLOR);

    // HALT
    while(1) {
        __asm__ volatile("cli; hlt");
    }
}

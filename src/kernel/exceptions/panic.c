#include "panic.h"
#include <klib/graphics/graphics.h>

__attribute__((noreturn)) void panic(const char *message)
{
    // Disable interrupts
    __asm__ volatile("cli");

    print("\n", GFX_WHITE);
    print("!!! --- KERNEL PANIC --- !!!", GFX_RED);
    print("\n", GFX_WHITE);

    if (message) {
        print(message, GFX_RED);
        print("\n", GFX_WHITE);
    }

    print("\nSystem halted.", GFX_WHITE);

    // HALT
    while(1) {
        __asm__ volatile("cli; hlt");
    }
}

__attribute__((noreturn)) void panic_exception(cpu_state_t *state, const char *message)
{
    // Disable interrupts
    __asm__ volatile("cli");

    print("\n", GFX_WHITE);
    print("!!! PANIC !!!", GFX_RED);
    print("\n", GFX_WHITE);

    if (message) {
        char buf[128];
        str_copy(buf, "Exception: ");
        str_append(buf, message);
        print(buf, GFX_RED);
        print("\n", GFX_WHITE);
    }

    // Print exception details
    char buf[128];
    str_copy(buf, "INT: ");
    str_append_uint(buf, (u32)state->int_no);
    str_append(buf, " ERR: ");
    str_append_uint(buf, (u32)state->err_code);
    print(buf, GFX_YELLOW);
    print("\n", GFX_WHITE);

    // Print RIP
    str_copy(buf, "RIP: 0x");
    str_append_uint(buf, (u32)(state->rip >> 32));
    str_append_uint(buf, (u32)(state->rip & 0xFFFFFFFF));
    print(buf, GFX_YELLOW);
    print("\n", GFX_WHITE);

    // Print RSP
    str_copy(buf, "RSP: 0x");
    str_append_uint(buf, (u32)(state->rsp >> 32));
    str_append_uint(buf, (u32)(state->rsp & 0xFFFFFFFF));
    print(buf, GFX_YELLOW);
    print("\n\n", GFX_WHITE);

    print("System halted.", GFX_WHITE);

    // HALT
    while(1) {
        __asm__ volatile("cli; hlt");
    }
}

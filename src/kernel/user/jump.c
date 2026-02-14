#include "ulime.h"

#include <kernel/arch/x86_64/gdt/gdt.h>
#include <kernel/communication/serial.h>
#include <theme/stdclrs.h>

void JumpToUserspace(ulime_proc_t *proc) {
    if (!proc) {
        printf("[ULIME] error: cannot jump to null process\n");
        return;
    }

    printf("[ULIME] jumping to userspace: %s (pid=%lu)\n", proc->name, proc->pid);
    printf("  entry: 0x%lX\n", proc->entry_point);
    printf("  stack: 0x%lX\n", proc->stack_base + proc->stack_size);

    u64 user_rip = proc->entry_point;
    u64 user_rsp = proc->stack_base + proc->stack_size - 16;
    u64 user_rflags = 0x202;

    u16 user_cs = USER_CODE_SELECTOR | 3;
    u16 user_ss = USER_DATA_SELECTOR | 3;

    proc->state = PROC_RUNNING;

    // DEBUG: Ausgabe aller Werte
    printf("  CS: 0x%X, SS: 0x%X\n", user_cs, user_ss);
    printf("  RIP: 0x%lX, RSP: 0x%lX, RFLAGS: 0x%lX\n",
           user_rip, user_rsp, user_rflags);

    // DEBUG: CR3 ausgeben
    u64 cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    printf("  CR3: 0x%lX\n", cr3);

    // DEBUG: Prüfe ob Entry Point gemappt ist
    u8 *code = (u8*)user_rip;
    printf("  Code at entry: %02X %02X %02X %02X\n",
           code[0], code[1], code[2], code[3]);

    printf("[ULIME] executing iretq...\n");

    __asm__ volatile(
        "push %0\n"
        "push %1\n"
        "push %2\n"
        "push %3\n"
        "push %4\n"
        "iretq\n"
        :
        : "r"((u64)user_ss),
          "r"(user_rsp),
          "r"(user_rflags),
          "r"((u64)user_cs),
          "r"(user_rip)
        : "memory"
    );


    // should never reach here
    __builtin_unreachable();
}

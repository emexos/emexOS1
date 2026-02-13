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
    u64 user_rsp = proc->stack_base + proc->stack_size - 16;  // 16-byte alignment
    u64 user_rflags = 0x202;  // IF (interrupts enabled) + reserved bit 1

    // prepare segments
    u16 user_cs = USER_CODE_SELECTOR | 3;  // RPL=3
    u16 user_ss = USER_DATA_SELECTOR | 3;  // RPL=3

    // set current process
    proc->state = PROC_RUNNING;

    // iretq not iret if im not wrong.......
    __asm__ volatile(
        //"mov %0, %%rsp\n"// switch to user stack temporarily
        "push %1\n" // SS (user data segment)
        "push %2\n" // RSP
        "push %3\n" // RFLAGS
        "push %4\n" // CS
        "push %5\n" // RIP
        "iretq\n"
        :
        : "r"(user_rsp),
          "r"((u64)user_ss),
          "r"(user_rsp),
          "r"(user_rflags),
          "r"((u64)user_cs),
          "r"(user_rip)
        : "memory"
    );


    // should never reach here
    __builtin_unreachable();
}

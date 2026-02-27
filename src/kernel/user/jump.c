#include "ulime.h"

#include <kernel/arch/x86_64/gdt/gdt.h>
#include <kernel/communication/serial.h>
#include <kernel/mem/paging/paging.h>
#include <theme/stdclrs.h>

void JumpToUserspace(ulime_proc_t *proc) {
    if (!proc) {
        printf("[ULIME] error: cannot jump to null process\n");
        return;
    }

    //u64 kernel_stack = proc->stack_base + 0x1000;
    //gdt_set_kernel_stack(kernel_stack);

    u64 user_rip = proc->entry_point;
    u64 user_rsp = (proc->stack_base + proc->stack_size - 16) & ~0xFULL;  // 16-byte aligned
    u64 user_rflags = 0x202;  // IF=1 , reserved bit 1 set

    u16 user_cs = USER_CODE_SELECTOR | 3; // RPL=3
    u16 user_ss = USER_DATA_SELECTOR | 3; // RPL=3

    proc->state = PROC_RUNNING;

    printf("[ULIME] Jumping to %s at RIP=0x%lX RSP=0x%lX\n",
           proc->name, user_rip, user_rsp);

    // switch to the processes own PML4 BEFORE touching any userspace addresses
    __asm__ volatile( "mov %0, %%cr3" : : "r"(proc->pml4_phys) : "memory");
    __asm__ volatile(
        "cli\n"

        "subq $40, %%rsp\n"//allocate 40 bytes

        "movq %0, 32(%%rsp)\n" // SS
        "movq %1, 24(%%rsp)\n" // user RSP
        "movq %2, 16(%%rsp)\n" // RFLAGS
        "movq %3, 8(%%rsp)\n" // CS
        "movq %4, 0(%%rsp)\n" // RIP

        // verifys what debug wrote (halt if after jump back doesnt got verified)
        //"movq 0(%%rsp), %%rax\n"
        //"cmp %4, %%rax\n"
        //"jne 1f\n"

        //"cli\n"
        // clears General Purpose Registers
        "xor %%rax, %%rax\n"
        "xor %%rbx, %%rbx\n"
        "xor %%rcx, %%rcx\n"
        "xor %%rdx, %%rdx\n"
        "xor %%rsi, %%rsi\n"
        "xor %%rdi, %%rdi\n"
        "xor %%r8, %%r8\n"
        "xor %%r9, %%r9\n"
        "xor %%r10, %%r10\n"
        "xor %%r11, %%r11\n"
        "xor %%r12, %%r12\n"
        "xor %%r13, %%r13\n"
        "xor %%r14, %%r14\n"
        "xor %%r15, %%r15\n"
        "xor %%rbp, %%rbp\n"

        // even sysretq doesnt work
        "iretq\n"

        //"1:\n"  // corruption
        //"hlt\n"
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

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

    u64 kernel_stack = proc->stack_base + 0x1000;
    gdt_set_kernel_stack(kernel_stack);

    u64 user_rip = proc->entry_point;
    u64 user_rsp = (proc->stack_base + proc->stack_size - 16) & ~0xFULL;  // 16-byte aligned
    u64 user_rflags = 0x202;  // IF=1 , reserved bit 1 set

    u16 user_cs = USER_CODE_SELECTOR | 3; // RPL=3
    u16 user_ss = USER_DATA_SELECTOR | 3; // RPL=3

    proc->state = PROC_RUNNING;

    printf("[ULIME] Jumping to %s at RIP=0x%lX RSP=0x%lX\n",
           proc->name, user_rip, user_rsp);

    if (user_rip < proc->heap_base || user_rip >= proc->heap_base + proc->heap_size) {
        printf("[ULIME] ERROR: RIP 0x%lX outside process heap!\n", user_rip);
        return;
    }
    if (user_rsp < proc->stack_base || user_rsp >= proc->stack_base + proc->stack_size) {
        printf("[ULIME] ERROR: RSP 0x%lX outside process stack!\n", user_rsp);
        return;
    }

    printf("[ULIME] Values: SS=0x%X CS=0x%X RFLAGS=0x%lX\n",
           user_ss, user_cs, user_rflags);

    printf("[ULIME] iretq frame will be:\n");
    printf("  [RIP] = 0x%016lX\n", user_rip);
    printf("  [CS]  = 0x%016lX\n", (u64)user_cs);
    printf("  [RFLAGS] = 0x%016lX\n", user_rflags);
    printf("  [RSP] = 0x%016lX\n", user_rsp);
    printf("  [SS]  = 0x%016lX\n", (u64)user_ss);

    u64 *test_rip = (u64*)user_rip;
    u64 *test_rsp = (u64*)user_rsp;

    printf("[ULIME] Testing memory access:\n");
    printf("  Code at RIP: 0x%016lX\n", *test_rip);

    // try writing to stack
    *test_rsp = 0xDEADBEEF;
    if (*test_rsp != 0xDEADBEEF) {
        printf("[ULIME] ERROR: Stack not writable!\n");
        return;
    }
    printf("  Stack test: PASS\n");

    // page table permissions
    printf("\n");
    if (!verify_page_permissions(proc->ulime->hpr, user_rip, "Code page")) {
        printf("[ULIME] ERROR: code page not properly mapped!\n");
        return;
    }
    printf("\n");
    if (!verify_page_permissions(proc->ulime->hpr, user_rsp, "Stack page")) {
        printf("[ULIME] ERROR: stack page not properly mapped!\n");
        return;
    }
    printf("\n");

    __asm__ volatile(
        "cli\n"

        "subq $40, %%rsp\n"//$40 == (allocate) 40 bytes

        "movq %0, 32(%%rsp)\n" // SS
        "movq %1, 24(%%rsp)\n" // user RSP
        "movq %2, 16(%%rsp)\n" // RFLAGS
        "movq %3, 8(%%rsp)\n" // CS
        "movq %4, 0(%%rsp)\n" // RIP

        // verifys what debug wrote (halt if after jump back doesnt got verified)
        "movq 0(%%rsp), %%rax\n"
        "cmp %4, %%rax\n"
        "jne 1f\n"

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

        "1:\n"  // corruption
        "hlt\n"
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

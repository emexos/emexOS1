#include "syscalls.h"
#include "calls.h"
#include <kernel/communication/serial.h>
#include <kernel/arch/x86_64/exceptions/panic.h>
#include <memory/main.h>
#include <string/string.h>
#include <kernel/mem/paging/paging.h>
#include <kernel/graph/theme.h>
#include <theme/doccr.h>

// syscall handlers
u64 scall_write(ulime_proc_t *proc, u64 fd, u64 buf, u64 count) {
    (void)proc;
    (void)fd;

    const char *str = (const char *)buf;
    for (u64 i = 0; i < count; i++) {
        //count ++;
        BOOTUP_PRINTF("%c", str[i]);
    }

    return count;
}

u64 scall_exit(ulime_proc_t *proc, u64 exit_code, u64 arg2, u64 arg3) {
    (void)arg2;
    (void)arg3;

    BOOTUP_PRINTF("\n[USERMODE] process '%s' exited with code %lu\n",
           proc->name, exit_code);
    proc->state = PROC_ZOMBIE;

    BOOTUP_PRINTF("[USERMODE] !!! RETURN !!!\n");

    while(1) {
        __asm__ volatile("cli; hlt");
    }

    return 0;
}

void _init_syscalls(ulime_t *ulime) {
    memset(ulime->syscalls, 0, sizeof(ulime->syscalls));

    ulime->syscalls[EXIT]   = scall_exit;
    ulime->syscalls[WRITE]  = scall_write;
    //ulime->syscalls[READ]   = call_read;
    //ulime->syscalls[__GETPID] = call_getpid;

    printf("[USYS] syscalls initialized\n"); // ulime syscalls
}

#include "user.h"
#include "calls.h"

#include <kernel/communication/serial.h>
//#include <kernel/arch/x86_64/exceptions/panic.h>
#include <memory/main.h>
#include <string/string.h>
//#include <kernel/mem/paging/paging.h>
#include <kernel/graph/theme.h>
#include <theme/doccr.h>

static ulime_t *g_ulime = NULL;

// syscall handlers
u64 scall_write(ulime_proc_t *proc, u64 fd, u64 buf, u64 count) {
    (void)proc;
    (void)fd;

    const char *str = (const char *)buf;
    for (u64 i = 0; i < count; i++) {
        //count ++;
        printf("%c", str[i]);
    }

    return count;
}

u64 scall_exit(ulime_proc_t *proc, u64 exit_code, u64 arg2, u64 arg3) {
    (void)arg2;
    (void)arg3;

    printf("\n[SYSCALL] process '%s' exited with code %lu\n",
           proc->name, exit_code);
    proc->state = PROC_ZOMBIE;

    //BOOTUP_PRINTF("[USERMODE] !!! RETURN !!!\n");

    while(1) {
        __asm__ volatile("cli; hlt");
    }

    return 0;
}

/*u64 scall_read(ulime_proc_t *proc, u64 fd, u64 buf, u64 count) {
    (void)proc;
    (void)fd;
    (void)buf;
    (void)count;

    return 0;
}*/

u64 scall_getpid(ulime_proc_t *proc, u64 arg1, u64 arg2, u64 arg3) {
    (void)arg1;
    (void)arg2;
    (void)arg3;

    return proc->pid;
}

u64 scall_brk(ulime_proc_t *proc, u64 addr, u64 arg2, u64 arg3) {
    (void)arg2;
    (void)arg3;

    if (addr == 0) {
        return proc->heap_base + proc->heap_size;
    }

    u64 new_size = addr - proc->heap_base;
    if (new_size > proc->heap_size) {
        proc->heap_size = new_size;
    }

    return proc->heap_base + proc->heap_size;
}

// initialize syscall table
void _init_syscalls_table(ulime_t *ulime) {
    if (!ulime) return;

    g_ulime = ulime;
    memset(ulime->syscalls, 0, sizeof(ulime->syscalls));

    ulime->syscalls[EXIT]   = scall_exit;
    ulime->syscalls[WRITE]  = scall_write;
    //ulime->syscalls[READ]   = scall_read;
    ulime->syscalls[GETPID] = scall_getpid;
    ulime->syscalls[BRK]    = scall_brk;

    log("[SYSCALL]", "syscall table initialized\n", d);
}

// syscall handler (called from assembly)
u64 syscall_handler(u64 syscall_num, u64 arg1, u64 arg2, u64 arg3) {
    if (!g_ulime) {
        printf("[SYSCALL] error: ulime not initialized\n");
        return (u64)-1;
    }

    ulime_proc_t *current = g_ulime->ptr_proc_curr;
    if (!current) {
        printf("[SYSCALL] error: no current process\n");
        return (u64)-1;
    }

    if (syscall_num >= 256 || !g_ulime->syscalls[syscall_num]) {
        printf("[SYSCALL] unknown syscall %lu\n", syscall_num);
        return (u64)-1;
    }

    return g_ulime->syscalls[syscall_num](current, arg1, arg2, arg3);
}

#include "user.h"
#include "calls.h"

#include <kernel/communication/serial.h>
//#include <kernel/arch/x86_64/exceptions/panic.h>
#include <memory/main.h>
#include <string/string.h>
//#include <kernel/mem/paging/paging.h>
#include <kernel/graph/theme.h>
#include <kernel/console/graph/print.h>
#include <kernel/packages/emex/emex.h>
#include <theme/doccr.h>

// read syscall
#include <drivers/ps2/keyboard/keyboard.h>

static ulime_t *g_ulime = NULL;

// syscall handlers
u64 scall_write(ulime_proc_t *proc, u64 fd, u64 buf, u64 count) {
    (void)proc;
    if (fd != 1 && fd != 2) return (u64)-1;

    // validate buf is in userspace range (basic sanity check)
    if (buf == 0 || buf > 0x0000800000000000ULL) return (u64)-1;

    const char *str = (const char *)buf;

    // write each char to screen using console output
    for (u64 i = 0; i < count; i++) {
        char tmp[2];
        tmp[0] = str[i];
        tmp[1] = '\0';
        cprintf(tmp, 0xFFFFFFFF); // white
    }

    return count;
}

u64 scall_exit(ulime_proc_t *proc, u64 exit_code, u64 arg2, u64 arg3) {
    (void)arg2;
    (void)arg3;

    printf("\n[SYSCALL] process '%s' exited with code %lu\n", proc->name, exit_code);
    proc->state = PROC_ZOMBIE;

    //BOOTUP_PRINTF("[USERMODE] !!! RETURN !!!\n");
    // clear current process so scheduler doesn't try to return to it
    if (g_ulime) {
        g_ulime->ptr_proc_curr = NULL;
    }

    while(1) {
        __asm__ volatile("cli; hlt");
    }

    return 0;
}

u64 scall_read(ulime_proc_t *proc, u64 fd, u64 buf, u64 count) {
    (void)proc;
    //(void)fd;
    //(void)buf;
    //(void)count;

    // only stdin supported
    if (fd != 0) return (u64)-1;
    if (buf == 0 || count == 0) return 0;

    char *out = (char *)buf;
    u64 i = 0;

    // reenable interrupts
    __asm__ volatile("sti");

    while (i < count - 1) {
        // TODO:
        // use semaphores or spinlocks NOT busywaits
        while (!keyboard_has_key()) {
            __asm__ volatile("hlt");
        }

        key_event_t event;
        if (!keyboard_get_event(&event)) continue;
        if (!event.pressed) continue;

        char c = (char)(event.keycode & 0xFF);

        if (c == '\n' || c == '\r') {
            char nl[2] = {'\n', '\0'};
            cprintf(nl, 0xFFFFFFFF);
            out[i++] = '\n';
            break;
        }

        if (c == '\b') {
            if (i > 0) {
                i--;
                cprintf("\b", 0xFFFFFFFF);
            }
            continue;
        }

        if (c < 0x20 || c > 0x7E) continue;

        char echo[2] = {c, '\0'};
        cprintf(echo, 0xFFFFFFFF);
        out[i++] = c;
    }

    // i think this is slow........ this read syscall :(
    __asm__ volatile("cli");

    out[i] = '\0';
    return i;
}

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

u64 scall_execve(ulime_proc_t *proc, u64 path_ptr, u64 arg2, u64 arg3) {
    //(void)proc;
    (void)arg2;
    (void)arg3;

    // validate pointer
    if (path_ptr == 0 || path_ptr > 0x0000800000000000ULL) return (u64)-1;

    const char *path = (const char *)path_ptr;

    printf("[SYSCALL] execve: '%s'\n", path);

    // emex_launch_app creates the process
    // the scheduler will switch to it on the next quantum.
    ulime_proc_t *new_proc = NULL;
    int result = emex_launch_app(path, &new_proc);
    if (result != 0 || !new_proc) {
        printf("[SYSCALL] execve failed with code %d\n", result);
        return (u64)-1;
    }

    // replace current process, mark it as zombie
    proc->state = PROC_ZOMBIE;
    g_ulime->ptr_proc_curr = new_proc;

    // jump directly to the new process — never returns (iretq)
    JumpToUserspace(new_proc);

    __builtin_unreachable();
}

void _init_syscalls_table(ulime_t *ulime) {
    if (!ulime) return;

    g_ulime = ulime;
    memset(ulime->syscalls, 0, sizeof(ulime->syscalls));

    ulime->syscalls[READ]   = scall_read;
    ulime->syscalls[WRITE]  = scall_write;
    //ulime->syscalls[READ]   = scall_read;
    ulime->syscalls[GETPID] = scall_getpid;
    ulime->syscalls[BRK]    = scall_brk;
    ulime->syscalls[EXIT]   = scall_exit;
    ulime->syscalls[EXECVE] = scall_execve;

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
        printf("[SYSCALL] error: unknown syscall %lu\n", syscall_num);
        return (u64)-1;
    }

    return g_ulime->syscalls[syscall_num](current, arg1, arg2, arg3);
}

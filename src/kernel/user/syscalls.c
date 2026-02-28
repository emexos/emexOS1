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
#include <kernel/packages/elf/loader.h>
#include <kernel/file_systems/vfs/vfs.h>
#include <kernel/proc/proc_manager.h>
#include <theme/doccr.h>
#include <kernel/arch/x86_64/gdt/gdt.h>

// read syscall
#include <drivers/ps2/keyboard/keyboard.h>


static ulime_t *g_ulime = NULL;

#if ENABLE_ULIME
extern ulime_t *ulime;
#endif

#define SCWRITE 0xFFFFFFFF

// fromsyscall_entry.asm
extern u64 user_rsp;
extern u64 user_rcx;
extern u64 user_r11;
extern u64 user_cr3;
extern u64 user_rbx;
extern u64 user_rbp;
extern u64 user_r12;
extern u64 user_r13;
extern u64 user_r14;
extern u64 user_r15;
extern void resume_parent_sysret(u64 rip,u64 rsp,u64 r11,u64 cr3,u64 rbx,u64 rbp,u64 r12,u64 r13,u64 r14,u64 r15);

// full saved context of the blocked parent
static ulime_proc_t *blocked_parent = NULL;
static u64 blocked_parent_rip = 0;
static u64 blocked_parent_rsp = 0;
static u64 blocked_parent_cr3 = 0;
static u64 blocked_parent_rbx = 0;
static u64 blocked_parent_r11 = 0;
static u64 blocked_parent_rbp = 0;
static u64 blocked_parent_r12 = 0;
static u64 blocked_parent_r13 = 0;
static u64 blocked_parent_r14 = 0;
static u64 blocked_parent_r15 = 0;
static ulime_proc_t *find_proc_by_cr3(ulime_t *u, u64 cr3) {
    ulime_proc_t *p = u->ptr_proc_list;
    while (p) {
        if (p->pml4_phys == cr3) return p;
        p = p->next;
    }
    return NULL;
}

static u32 ansi_fg_color = SCWRITE;
static u32 ansi_code_to_color(int code) {
    switch (code) {
        case  0: return 0xFFFFFFFF; // reset (white)
        case 30: return 0xFF111111; // black
        case 31: return 0xFFFF5555; // red
        case 32: return 0xFF55FF55; // green
        case 33: return 0xFFFFFF55; // yellow
        case 34: return 0xFF5555FF; // blue
        case 35: return 0xFFFF55FF; // magenta
        case 36: return 0xFF55FFFF; // cyan
        case 37: return 0xFFFFFFFF; // white
        case 90: return 0xFF888888; // gray
        case 91: return 0xFFFF8888; // bright red
        case 92: return 0xFF88FF88; // bright green
        case 93: return 0xFFFFFF88; // bright yellow
        case 94: return 0xFF8888FF; // bright blue
        case 95: return 0xFFFF88FF; // bright magenta
        case 96: return 0xFF88FFFF; // bright cyan
        case 97: return 0xFFFFFFFF; // bright white
        default: return 0xFFFFFFFF;
    }
}

typedef enum {
    ANSI_STATE_NORMAL = 0,
    ANSI_STATE_ESC,// saw \033
    ANSI_STATE_CSI,// saw \033[
} ansi_state_t;

static ansi_state_t ansi_state = ANSI_STATE_NORMAL;
static int          ansi_param = 0;

static void ansi_write_char(char c) {
    char tmp[2] = {c, '\0'};
    switch (ansi_state) {
        case ANSI_STATE_NORMAL:
            if (c == '\033') { ansi_state = ANSI_STATE_ESC; }
            else              { cprintf(tmp, ansi_fg_color); }
            break;
        case ANSI_STATE_ESC:
            if (c == '[') { ansi_state = ANSI_STATE_CSI; ansi_param = 0; }
            else          { cprintf("\033", ansi_fg_color); cprintf(tmp, ansi_fg_color);
                            ansi_state = ANSI_STATE_NORMAL; }
            break;
        case ANSI_STATE_CSI:
            if (c >= '0' && c <= '9') {
                ansi_param = ansi_param * 10 + (c - '0');
            } else if (c == 'm') {
                ansi_fg_color = ansi_code_to_color(ansi_param);
                ansi_param    = 0;
                ansi_state    = ANSI_STATE_NORMAL;
            } else if (c == ';') {
                ansi_fg_color = ansi_code_to_color(ansi_param);
                ansi_param    = 0;
            } else {
                ansi_param = 0;
                ansi_state = ANSI_STATE_NORMAL;
            }
            break;
    }
}

// syscall handlers
u64 scall_write(ulime_proc_t *proc, u64 fd, u64 buf, u64 count) {
    (void)proc;
    if (fd != 1 && fd != 2) return (u64)-1;

    // validate buf is in userspace range
    if (buf == 0 || buf > 0x0000800000000000ULL) return (u64)-1;

    const char *str = (const char *)buf;

    // write each char to screen using console output
    for (u64 i = 0; i < count; i++) {
        ansi_write_char(str[i]);
    }
    return count;
}

u64 scall_exit(ulime_proc_t *proc, u64 exit_code, u64 arg2, u64 arg3) {
    (void)arg2;
    (void)arg3;

    printf("\n[SYSCALL] process '%s' exited with code %lu\n", proc->name, exit_code);
    proc->state = PROC_ZOMBIE;

    if (blocked_parent) {
        ulime_proc_t *parent = blocked_parent;
        u64 rip = blocked_parent_rip;
        u64 rsp = blocked_parent_rsp;
        u64 r11 = blocked_parent_r11;
        u64 cr3 = blocked_parent_cr3;
        u64 rbx = blocked_parent_rbx;
        u64 rbp = blocked_parent_rbp;
        u64 r12 = blocked_parent_r12;
        u64 r13 = blocked_parent_r13;
        u64 r14 = blocked_parent_r14;
        u64 r15 = blocked_parent_r15;

        blocked_parent = NULL;
        blocked_parent_rip = 0;
        blocked_parent_rsp = 0;
        blocked_parent_r11 = 0;
        blocked_parent_cr3 = 0;
        blocked_parent_rbx = 0;
        blocked_parent_rbp = 0;
        blocked_parent_r12 = 0;
        blocked_parent_r13 = 0;
        blocked_parent_r14 = 0;
        blocked_parent_r15 = 0;

        parent->state = PROC_RUNNING;
        g_ulime->ptr_proc_curr = parent;

        printf("[SYSCALL] resuming '%s' via sysret: RIP=0x%lX\n", parent->name, rip);

        // restore full parent state
        resume_parent_sysret(rip, rsp, r11, cr3, rbx, rbp, r12, r13, r14, r15);
        __builtin_unreachable();
    }

    g_ulime->ptr_proc_curr = NULL;
    while(1) {
        __asm__ volatile("cli; hlt");
    }
    return 0;
}

static void setup_argv_on_stack(ulime_proc_t *new_proc, char **user_argv, int argc) {
    if (argc <= 0 || !user_argv) return;

    u64 hhdm   = g_ulime->hpr->offset;
    u64 pstack = new_proc->phys_stack;
    u64 ssize  = new_proc->stack_size;
    u64 sbase  = new_proc->stack_base;

    u64 str_p_phys = pstack + ssize;
    u64 str_p_virt = sbase  + ssize;

    u64 str_vaddrs[32];

    for (int i = argc - 1; i >= 0; i--) {
        const char *s = user_argv[i];
        u64 len = 0;
        while (s[len]) len++;
        len++;

        str_p_phys -= len;
        str_p_virt -= len;

        char *dst = (char *)(hhdm + str_p_phys);
        for (u64 j = 0; j < len; j++) dst[j] = s[j];
        str_vaddrs[i] = str_p_virt;
    }

    str_p_virt = str_p_virt & ~7ULL;
    str_p_phys = pstack + (str_p_virt - sbase);

    u64 ptrsec   = (u64)(1 + argc + 1) * 8;
    u64 rsp_phys = (str_p_phys - ptrsec) & ~0xFULL;
    u64 rsp_virt = sbase + (rsp_phys - pstack);

    u64 *p = (u64 *)(hhdm + rsp_phys);
    *p++ = (u64)argc;
    for (int i = 0; i < argc; i++) *p++ = str_vaddrs[i];
    *p = 0;

    new_proc->entry_rsp = rsp_virt;
}

u64 scall_execve(ulime_proc_t *proc, u64 path_ptr, u64 argv_ptr, u64 arg3) {
    (void)arg3;

    if (path_ptr == 0 || path_ptr > 0x0000800000000000ULL) return (u64)-1;

    const char *path = (const char *)path_ptr;

    printf("[SYSCALL] execve: '%s'\n", path);

    ulime_proc_t *caller = find_proc_by_cr3(g_ulime, user_cr3);
    if (!caller) {caller = proc;}

    int path_len = str_len(path);
    int is_elf = (path_len > 4 &&
                  path[path_len - 4] == '.' &&
                  path[path_len - 3] == 'e' &&
                  path[path_len - 2] == 'l' &&
                  path[path_len - 1] == 'f');

    ulime_proc_t *new_proc = NULL;

    if (is_elf) {
        if (!ulime || !proc_mgr) {
            printf("[SYSCALL] execve: ulime not ready\n");
            return (u64)-1;
        }

        int fd = fs_open(path, O_RDONLY);
        if (fd < 0) {
            printf("[SYSCALL] execve: cannot open '%s'\n", path);
            return (u64)-1;
        }

        #define EXECVE_ELF_MAX (512 * 1024)
        static u8 execve_elf_buf[EXECVE_ELF_MAX];

        ssize_t elf_size = fs_read(fd, execve_elf_buf, EXECVE_ELF_MAX);
        fs_close(fd);

        if (elf_size <= 0) {
            printf("[SYSCALL] execve: elf empty or unreadable\n");
            return (u64)-1;
        }

        const char *name = path;
        for (int i = 0; i < path_len; i++) {
            if (path[i] == '/') name = path + i + 1;
        }

        new_proc = proc_create_proc(proc_mgr, (u8*)name, 0, USERPRIORITY);
        if (!new_proc) {
            printf("[SYSCALL] execve: failed to create process\n");
            return (u64)-1;
        }

        if (elf_load(new_proc, execve_elf_buf, (u64)elf_size) != 0) {
            printf("[SYSCALL] execve: elf_load failed\n");
            return (u64)-1;
        }
    } else {

        int result = emex_launch_app(path, &new_proc);
        if (result != 0 || !new_proc) {
            printf("[SYSCALL] execve failed with code %d\n", result);
            return (u64)-1;
        }
    }

    if (argv_ptr && argv_ptr < 0x0000800000000000ULL) {
        char **user_argv = (char **)argv_ptr;
        int argc = 0;
        while (argc < 31 && user_argv[argc]) argc++;
        setup_argv_on_stack(new_proc, user_argv, argc);
    }

    blocked_parent     = caller;
    blocked_parent_rip = user_rcx;
    blocked_parent_rsp = user_rsp;
    blocked_parent_r11 = user_r11;
    blocked_parent_cr3 = user_cr3;
    blocked_parent_rbx = user_rbx;
    blocked_parent_rbp = user_rbp;
    blocked_parent_r12 = user_r12;
    blocked_parent_r13 = user_r13;
    blocked_parent_r14 = user_r14;
    blocked_parent_r15 = user_r15;

    caller->state = PROC_BLOCKED;
    g_ulime->ptr_proc_curr = new_proc;

    printf("[SYSCALL] process '%s' blocked, waiting for '%s'\n",
           caller->name, new_proc->name);

    JumpToUserspace(new_proc);

    __builtin_unreachable();
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
            cprintf(nl, SCWRITE);
            out[i++] = '\n';
            break;
        }
        if (c == '\b') {
            if (i > 0) {
                i--;
                cprintf("\b", SCWRITE);
            }
            continue;
        }

        if (c < 0x20 || c > 0x7E) continue;

        char echo[2] = {c, '\0'};
        cprintf(echo, SCWRITE);
        out[i++] = c;
    }

    // i think this is slow........ this read syscall :(
    __asm__ volatile("cli");

    out[i] = '\0';
    return i;
}

u64 scall_getpid(ulime_proc_t *proc, u64 arg1, u64 arg2, u64 arg3) {
    (void)arg1; (void)arg2; (void)arg3;
    return proc->pid;
}

u64 scall_brk(ulime_proc_t *proc, u64 addr, u64 arg2, u64 arg3) {
    (void)arg2; (void)arg3;

    if (addr == 0) return proc->brk;


    u64 heap_end = proc->heap_base + proc->heap_size;
    if (addr > heap_end) {
        printf("[SYSCALL] brk: OOM — 0x%lX > heap end 0x%lX\n", addr, heap_end);
        return proc->brk;
    }

    if (addr < proc->heap_base) return proc->brk;

    proc->brk = addr;
    return proc->brk;
}

u64 scall_open(ulime_proc_t *proc, u64 path_ptr, u64 flags, u64 arg3) {
    (void)proc; (void)arg3;
    if (!path_ptr || path_ptr > 0x0000800000000000ULL) return (u64)-1;
    int fd = fs_open((const char *)path_ptr, (int)flags);
    return (fd < 0) ? (u64)-1 : (u64)fd;
}

u64 scall_close(ulime_proc_t *proc, u64 fd, u64 arg2, u64 arg3) {
    (void)proc; (void)arg2; (void)arg3;
    return (u64)fs_close((int)fd);
}

u64 scall_getdents(ulime_proc_t *proc, u64 path_ptr, u64 buf_ptr, u64 max_entries) {
    (void)proc;
    if (!path_ptr || !buf_ptr || path_ptr > 0x0000800000000000ULL) return (u64)-1;
    int n = fs_listdir((const char *)path_ptr, (_emx_kdirent_t *)buf_ptr, (int)max_entries);
    return (n < 0) ? (u64)-1 : (u64)n;
}

void _init_syscalls_table(ulime_t *ulime_ptr) {
    if (!ulime_ptr) return;

    g_ulime = ulime_ptr;
    memset(ulime_ptr->syscalls, 0, sizeof(ulime_ptr->syscalls));

    ulime_ptr->syscalls[READ]     = scall_read;
    ulime_ptr->syscalls[WRITE]    = scall_write;
    //ulime->syscalls[READ]   = scall_read;
    ulime_ptr->syscalls[OPEN]     = scall_open;
    ulime_ptr->syscalls[CLOSE]    = scall_close;
    ulime_ptr->syscalls[GETPID]   = scall_getpid;
    ulime_ptr->syscalls[BRK]      = scall_brk;
    ulime_ptr->syscalls[EXIT]     = scall_exit;
    ulime_ptr->syscalls[EXECVE]   = scall_execve;
    ulime_ptr->syscalls[GETDENTS] = scall_getdents;

    log("[SYSCALL]", "syscall table initialized\n", d);
}

// syscall handler (called from assembly) uses find_proc_by_cr3
u64 syscall_handler(u64 syscall_num, u64 arg1, u64 arg2, u64 arg3) {
    if (!g_ulime) {
        printf("[SYSCALL] error: ulime not initialized\n");
        return (u64)-1;
    }

    ulime_proc_t *current = find_proc_by_cr3(g_ulime, user_cr3);
    if (!current) {
        printf("[SYSCALL] error: no process for CR3 0x%lX\n", user_cr3);
        return (u64)-1;
    }

    if (syscall_num >= 256 || !g_ulime->syscalls[syscall_num]) {
        printf("[SYSCALL] error: unknown syscall %lu\n", syscall_num);
        return (u64)-1;
    }

    return g_ulime->syscalls[syscall_num](current, arg1, arg2, arg3);
}

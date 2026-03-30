#include "user.h"
#include "calls.h"

#include <kernel/communication/serial.h>
//#include <kernel/arch/x86_64/exceptions/panic.h>
#include <memory/main.h>
#include <string/string.h>
//#include <kernel/mem/paging/paging.h>
#include <kernel/graph/theme.h>
#include <kernel/packages/emex/emex.h>
#include <kernel/packages/elf/loader.h>
#include <kernel/file_systems/vfs/vfs.h>
#include <kernel/proc/proc_manager.h>
#include <theme/doccr.h>
#include <kernel/arch/x86_64/gdt/gdt.h>
#include <kernel/arch/x86_64/exceptions/timer.h>
#include <kernel/cpu/cpu.h>

// read syscall
#include <drivers/drivers.h>

#include <kernel/devices/devices.h>

#include <config/user.h>

// memory
#include <kernel/user/mmap.h>
#include <kernel/mem/paging/paging.h>
#include <kernel/mem/phys/physmem.h>

//multitasking
#include <kernel/multitasking/ipc/ipc.h>
#include <kernel/multitasking/multitasking.h>

// sinfo
#include <kernel/user/sysinfo.h>

#include "scalls/scalls.h"

#include <kernel/user/ptrs.h>

//#include <kernel/devices/fb0/fb0.h>
//#include <kernel/devices/tty/tty0.h>


ulime_t *g_ulime = NULL;

#if ENABLE_ULIME
extern ulime_t *ulime;
#endif

#define SCWRITE 0xFFFFFFFF
//#define KEYBOARD0 KBDPATH
#define MOUSE0 MS0PATH

extern char cwd[];
extern mt_t *mt;

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
#define MAX_BLOCKED 16

typedef struct {
    ulime_proc_t *proc;
    u64 rip, rsp, r11, cr3, rbx, rbp, r12, r13, r14, r15;
    u64 waiting_for_pid;
} blocked_entry_t;

static blocked_entry_t blocked_list[MAX_BLOCKED];
static int blocked_count = 0;
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
            else { /*cprintf(tmp, ansi_fg_color);*/ }
            break;
        case ANSI_STATE_ESC:
            if (c == '[') { ansi_state = ANSI_STATE_CSI; ansi_param = 0; }
            else {/*cprintf("\033", ansi_fg_color); cprintf(tmp, ansi_fg_color);*/ ansi_state = ANSI_STATE_NORMAL;}
            break;
        case ANSI_STATE_CSI:
            if (c >= '0' && c <= '9') {
                ansi_param = ansi_param * 10 + (c - '0');
            } else if (c == 'm') {
                ansi_fg_color = ansi_code_to_color(ansi_param);
                ansi_param = 0;
                ansi_state = ANSI_STATE_NORMAL;
            } else if (c == ';') {
                ansi_fg_color = ansi_code_to_color(ansi_param);
                ansi_param = 0;
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
    if (!is_valid_user_ptr_range(buf, count)) return (u64)-1;

    if (fd >= 3)
        return (u64)fs_write((int)fd, (const void *)buf, (size_t)count);

    if (fd != 1 && fd != 2) return (u64)-1;

    // lazy-open /dev/tty0
    static int tty_fd = -1;
    if (tty_fd < 0) {
        tty_fd = fs_open(TTY0PATH, O_WRONLY);
        if (tty_fd < 0) {
            const char *s = (const char *)buf;
            for (u64 i = 0; i < count; i++) tty0_write_char(s[i]);
            return count;
        }
    }
    return (u64)fs_write(tty_fd, (const void *)buf, (size_t)count);
}

u64 scall_exit(ulime_proc_t *proc, u64 exit_code, u64 arg2, u64 arg3)
{
    (void)arg2;
    (void)arg3;

    printf("\n[SYSCALL] process '%s' exited with code %lu\n", proc->name, exit_code);
    proc->state = PROC_ZOMBIE;

    // remove from mt so timer skips it
    extern mt_t *mt;
    if (mt) {
        for (int i = 0; i < mt->task_count; i++) {
            if (mt->tasks[i].proc == proc) {
                mt->tasks[i].valid = 0;
                break;
            }
        }
    }

    // find who is waiting for this pid
    blocked_entry_t *found = NULL;
    int found_idx = -1;
    for (int i = 0; i < blocked_count; i++) {
        if (blocked_list[i].waiting_for_pid == proc->pid) {
            found = &blocked_list[i];
            found_idx = i;
            break;
        }
    }

    if (found) {
        ulime_proc_t *parent = found->proc;
        u64 rip = found->rip, rsp = found->rsp, r11 = found->r11;
        u64 cr3 = found->cr3, rbx = found->rbx, rbp = found->rbp;
        u64 r12 = found->r12, r13 = found->r13;
        u64 r14 = found->r14, r15 = found->r15;

        // remove from blocked list
        blocked_list[found_idx] = blocked_list[--blocked_count];

        parent->state = PROC_RUNNING;
        g_ulime->ptr_proc_curr = parent;

        if (mt) {
            for (int i = 0; i < mt->task_count; i++) {
                if (mt->tasks[i].proc == parent) {
                    mt->current_idx = i;
                    if (mt->tasks[i].kstack)
                        gdt_set_kernel_stack(((u64)mt->tasks[i].kstack + MT_KSTACK_SIZE) & ~0xFULL);
                    break;
                }
            }
        }

        // restore full parent state
        printf("[SYSCALL] resuming '%s' via sysret: RIP=0x%lX\n", parent->name, rip);
        resume_parent_sysret(rip, rsp, r11, cr3, rbx, rbp, r12, r13, r14, r15);
        __builtin_unreachable();
    }

    // one waitin
    if (mt) {
        for (int i = 0; i < mt->task_count; i++) {
            mt_task_t *t = &mt->tasks[i];
            if (!t->valid || !t->proc) continue;
            if (t->proc->state == PROC_ZOMBIE || t->proc->state == PROC_BLOCKED) continue;
            mt->current_idx = i;
            t->proc->state = PROC_RUNNING;
            g_ulime->ptr_proc_curr = t->proc;
            if (t->kstack) gdt_set_kernel_stack(((u64)t->kstack + MT_KSTACK_SIZE) & ~0xFULL);
            printf("[SYSCALL] no waiter, resuming '%s'\n", t->proc->name);
            if (t->user_ctx.saved) {
                resume_parent_sysret(
                    t->user_ctx.rip, t->user_ctx.rsp, t->user_ctx.rflags,
                    t->proc->pml4_phys,
                    t->user_ctx.rbx, t->user_ctx.rbp,
                    t->user_ctx.r12, t->user_ctx.r13,
                    t->user_ctx.r14, t->user_ctx.r15
                );
            } else {
                JumpToUserspace(t->proc);
            }
            __builtin_unreachable();
        }
    }

    printf("[SYSCALL] no more processes\n");
    for (;;) __asm__ volatile("hlt");
    return 0;
}

static void setup_argv_on_stack(ulime_proc_t *new_proc, char **user_argv, int argc)
{
    if (argc <= 0 || !user_argv) return;

    u64 hhdm = g_ulime->hpr->offset;
    u64 pstack = new_proc->phys_stack;
    u64 ssize = new_proc->stack_size;
    u64 sbase = new_proc->stack_base;

    u64 str_p_phys = pstack + ssize;
    u64 str_p_virt = sbase + ssize;

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

    u64 ptrsec =   (u64)(1 + argc + 1) * 8;
    u64 rsp_phys = (str_p_phys - ptrsec) & ~0xFULL;
    u64 rsp_virt = sbase + (rsp_phys - pstack);

    u64 *p = (u64*)(hhdm + rsp_phys);
    *p++ = (u64)argc;
    for (int i = 0; i < argc; i++) *p++ = str_vaddrs[i];
    *p = 0;

    new_proc->entry_rsp = rsp_virt;
}

u64 scall_execve(ulime_proc_t *proc, u64 path_ptr, u64 argv_ptr, u64 arg3)
{
    (void)arg3;

    if (!is_valid_user_ptr(path_ptr)) return (u64)-1;

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

    if (is_valid_user_ptr(argv_ptr)) {
        char **user_argv = (char **)argv_ptr;
        int argc = 0;
        while (argc < 31 && user_argv[argc]) argc++;
        setup_argv_on_stack(new_proc, user_argv, argc);
    }

    //blocked_parent     = caller;
    if (blocked_count < MAX_BLOCKED) {
        blocked_entry_t *e = &blocked_list[blocked_count++];
        e->proc = caller;
        e->rip = user_rcx;
        e->rsp = user_rsp;
        e->r11 = user_r11;
        e->cr3 = user_cr3;
        e->rbx = user_rbx;
        e->rbp = user_rbp;
        e->r12 = user_r12;
        e->r13 = user_r13;
        e->r14 = user_r14;
        e->r15 = user_r15;
        e->waiting_for_pid = new_proc->pid;
    }

    caller->state = PROC_BLOCKED;
    g_ulime->ptr_proc_curr = new_proc;

    if (mt) mt_add_task(mt, new_proc);

    printf(
    	"[SYSCALL] process '%s' blocked, waiting for '%s'\n",
            caller->name, new_proc->name
    );
    extern mt_t *mt;
    if (mt) {
        for (int i = 0; i < mt->task_count; i++) {
            if (mt->tasks[i].proc == new_proc) {
                mt->current_idx = i;
                if (mt->tasks[i].kstack)
                    gdt_set_kernel_stack(((u64)mt->tasks[i].kstack + MT_KSTACK_SIZE) & ~0xFULL);
                break;
            }
        }
    }

    JumpToUserspace(new_proc);

    __builtin_unreachable();
}

u64 scall_read(ulime_proc_t *proc, u64 fd, u64 buf, u64 count)
{
    (void)proc;
    if (buf == 0 || count == 0) return 0;

    // (urandom, zero, files, ...)
    if (fd >= 3)
        return (u64)fs_read((int)fd, (void *)buf, (size_t)count);

    // fd 0 stdin to tty0
    if (fd != 0) return (u64)-1;

    static int tty_fd = -1;
    if (tty_fd < 0) {
        tty_fd = fs_open(TTY0PATH, O_RDONLY);
        if (tty_fd < 0) {
            printf("[SYSCALL] read: cannot open " TTY0PATH "\n");
            return (u64)-1;
        }
    }
    ulime_proc_t *cur = find_proc_by_cr3(g_ulime, user_cr3);
    if (cur) cur->state = PROC_BLOCKED;
    u64 result = (u64)fs_read(tty_fd, (void *)buf, (size_t)count);
    if (cur) {
        cur->state = PROC_RUNNING;
        extern mt_t *mt;
        if (mt) {
            for (int i = 0; i < mt->task_count; i++) {
                if (mt->tasks[i].proc == cur) {
                    mt->current_idx = i;
                    break;
                }
            }
        }
    }
    return result;
}

u64 scall_getpid(ulime_proc_t *proc, u64 arg1, u64 arg2, u64 arg3) {
    (void)arg1; (void)arg2; (void)arg3;
    return proc->pid;
}

u64 scall_brk(ulime_proc_t *proc, u64 addr, u64 arg2, u64 arg3)
{
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
    if (!is_valid_user_ptr(path_ptr)) return (u64)-1;
    int fd = fs_open((const char *)path_ptr, (int)flags);
    return (fd < 0) ? (u64)-1 : (u64)fd;
}

u64 scall_close(ulime_proc_t *proc, u64 fd, u64 arg2, u64 arg3) {
    (void)proc; (void)arg2; (void)arg3;
    return (u64)fs_close((int)fd);
}

u64 scall_getdents(ulime_proc_t *proc, u64 path_ptr, u64 buf_ptr, u64 max_entries) {
    (void)proc;
    if (!is_valid_user_ptr(path_ptr) || !is_valid_user_ptr(buf_ptr)) return (u64)-1;
    int n = fs_listdir((const char *)path_ptr, (_emx_kdirent_t *)buf_ptr, (int)max_entries);
    return (n < 0) ? (u64)-1 : (u64)n;
}

u64 scall_chdir(ulime_proc_t *proc, u64 path_ptr, u64 arg2, u64 arg3)
{
    (void)proc; (void)arg2; (void)arg3;

    if (!is_valid_user_ptr(path_ptr)) return (u64)-1;

    const char *s = (const char *)path_ptr;

    // no argument then go to root
    if (!s || *s == '\0') {
        str_copy(cwd, "/");
        return 0;
    }

    char new_path[FS_MAX_PATH];

    // handle ".." (go back) before resolution
    if (str_equals(s, "..")) {
        int len = str_len(cwd);
        if (len > 1 && cwd[len - 1] == '/') {
            cwd[len - 1] = '\0';
            len--;
        }
        for (int i = len - 1; i >= 0; i--) {
            if (cwd[i] == '/') {
                if (i == 0) str_copy(cwd, "/");
                else cwd[i] = '\0';
                return 0;
            }
        }
        return 0;
    }

    // absolute vs relative path
    if (s[0] == '/') {
        str_copy(new_path, s);
    } else {
        str_copy(new_path, cwd);
        if (cwd[str_len(cwd) - 1] != '/') str_append(new_path, "/");
        str_append(new_path, s);
    }

    // verifyy the path exists and is a directory
    fs_node *dir = fs_resolve(new_path);
    if (!dir) return (u64)-1;
    if (dir->type != FS_DIR) return (u64)-1;


    // new cwd
    str_copy(cwd, new_path);

    // ensurey trailing slash (except for root)
    int len = str_len(cwd);
    if (len > 1 && cwd[len - 1] != '/') str_append(cwd, "/");

    return 0;
}

u64 scall_ioctl(ulime_proc_t *proc, u64 fd, u64 request, u64 arg_ptr)
{
    (void)proc;
    if (!is_valid_user_ptr(arg_ptr)) return (u64)-1;

    void *arg = (void *)arg_ptr;

    // fb0:fb0_ioctl
    if (fd >= 3) {
        fs_file *f = fs_get_file((int)fd);
        if (f && f->node) {
            // fb0 ioctl
            if (str_equals(f->node->name, "fb0")) {
                return (u64)fb0_ioctl((int)request, arg);
            }
        }
    }

    // tty ioctl (fd 0, 1, 2)
    if (fd <= 2) {
        switch ((int)request) {
            case 0: tty0_set_echo_mode(0); return 0; // TTY_ECHO
            case 1: tty0_set_echo_mode(1); return 0; // TTY_NOECHO
            case 2: tty0_set_echo_mode(2); return 0; // TTY_MASKECHO
            case 3: tty0_set_echo_mode(3); return 0; // TTY_RAW
            default: return (u64)-1;
        }
    }

    return (u64)-1;
}

u64 scall_fork(ulime_proc_t *proc, u64 arg1, u64 arg2, u64 arg3)
{
    (void)arg1; (void)arg2; (void)arg3;

    // null check
    if (!proc_mgr || !g_ulime) return (u64)-1;
    extern mt_t *mt;
    if (!mt) return (u64)-1;

    ulime_proc_t *child = proc_create_proc(proc_mgr, proc->name, proc->entry_point, proc->priority);
    if (!child) return (u64)-1;

    u64 hhdm = g_ulime->hpr->offset;

    // copy heap and stack content via hhdm
    memcpy((void*)(child->phys_heap + hhdm), (void*)(proc->phys_heap  + hhdm), proc->heap_size);
    memcpy((void*)(child->phys_stack + hhdm), (void*)(proc->phys_stack + hhdm), proc->stack_size);

    child->brk = child->heap_base + (proc->brk - proc->heap_base);
    child->entry_point = user_rcx;
    child->entry_rsp =   user_rsp;

    u64 heap_flags =  PTE_PRESENT | PTE_WRITABLE | PTE_USER;
    u64 stack_flags = PTE_PRESENT | PTE_WRITABLE | PTE_USER | PTE_NO_EXEC;
    u64 heap_pages =  proc->heap_size / 0x1000;
    u64 stack_pages = proc->stack_size / 0x1000;

    // map parents virtual address into childs PML4 so the copied stack return addresses work
    for (u64 p = 0; p < heap_pages; p++) {
        u64 parent_virt = proc->heap_base + (p * 0x1000);
        u64 child_phys = child->phys_heap + (p * 0x1000);
        paging_map_page_proc(g_ulime->hpr, child->pml4_phys, parent_virt, child_phys, heap_flags);
    }
    for (u64 p = 0; p < stack_pages; p++) {
        u64 parent_virt = proc->stack_base + (p * 0x1000);
        u64 child_phys = child->phys_stack + (p * 0x1000);
        paging_map_page_proc(g_ulime->hpr, child->pml4_phys, parent_virt, child_phys, stack_flags);
    }

    if (proc->vma_base != 0) {
        for (u64 p = 0; p < heap_pages; p++) {
            u64 orig_virt = proc->vma_base + (p * 0x1000);
            u64 child_phys = child->phys_heap + (p * 0x1000);
            paging_map_page_proc(g_ulime->hpr, child->pml4_phys, orig_virt, child_phys, heap_flags);
        }
    }
    child->vma_base = proc->vma_base;

    //extern mt_t *mt;
    int idx = mt_add_task(mt, child);
    //if (!mt) return (u64)-1;
    if (idx < 0) return (u64)-1;

    // save full child context so timer can switch to it correctly
    mt_task_t *ct = &mt->tasks[idx];
    ct->user_ctx.rip = user_rcx; // resume after fork() syscall
    ct->user_ctx.rsp = user_rsp;
    ct->user_ctx.cs = (USER_CODE_SELECTOR | 3);
    ct->user_ctx.rflags = 0x202; // IF=1
    ct->user_ctx.ss = (USER_DATA_SELECTOR | 3);
    ct->user_ctx.cr3 = child->pml4_phys;
    ct->user_ctx.rax = 0; // child sees fork() == 0
    ct->user_ctx.rbx = user_rbx;
    ct->user_ctx.rbp = user_rbp;
    ct->user_ctx.r12 = user_r12;
    ct->user_ctx.r13 = user_r13;
    ct->user_ctx.r14 = user_r14;
    ct->user_ctx.r15 = user_r15;
    ct->user_ctx.saved = 1;

    printf("[SYSCALL] fork: parent pid=%llu child pid=%llu\n", proc->pid, child->pid);

    return child->pid;  // parent sees child pid
}

u64 scall_getcwd(ulime_proc_t *proc, u64 buf_ptr, u64 size, u64 arg3) {
    (void)proc;
    (void)arg3;

    if (!is_valid_user_ptr_range(buf_ptr, size)) return 0;

    char *buf = (char *)buf_ptr;
    int cwlen = str_len(cwd);

    if ((u64)(cwlen + 1) > size) return 0; // buffer too small

    str_copy(buf, cwd);
    return (u64)(cwlen + 1); // return the written byte count
}

/*u64 scall_mmap(ulime_proc_t *proc, u64 args_ptr, u64 arg2, u64 arg3) {
    (void)arg2; (void)arg3;

    if (!args_ptr || args_ptr > 0x0000800000000000ULL) return (u64)MAP_FAILED;

    mmap_args_t *a = (mmap_args_t *)args_ptr;

    if (a->length == 0) return (u64)MAP_FAILED;

    u64 len   = (a->length + 0xFFF) & ~0xFFFULL;
    u64 pages = len / 4096;

    if (a->flags & MAP_ANONYMOUS) {
        u64 phys = physmem_alloc_to(pages);
        if (!phys) return (u64)MAP_FAILED;

        u64 virt = a->addr;
        if (!virt || virt < 0x400000) {
            virt = proc->mmap_base;
            if (!virt) virt = proc->heap_base + proc->heap_size + 0x100000;
        }
        virt = (virt + 0xFFF) & ~0xFFFULL;

        for (u64 i = 0; i < pages; i++) {
            paging_map_page_proc(
                g_ulime->hpr,
                proc->pml4_phys,
                virt + i * 4096,
                phys + i * 4096,
                USER_FLAGS
            );
        }

        // zero via HHDM
        u8 *p = (u8 *)(phys + g_ulime->hpr->offset);
        for (u64 i = 0; i < len; i++) p[i] = 0;

        proc->mmap_base = virt + len;
        return virt;
    }

    // fb0 file mapping
    if (a->fd >= 3) {
        fs_file *f = fs_get_file(a->fd);
        if (f && f->node && str_equals(f->node->name, "fb0")) {
            u32 *fb    = get_framebuffer();
            u64 fbphys = virt_to_phys(g_ulime->hpr, (void *)fb);
            u64 fbsize = (u64)get_fb_pitch() * get_fb_height();

            u64 fbpages = (fbsize + 0xFFF) / 4096;
            u64 virt    = a->addr;
            if (!virt) virt = proc->mmap_base;
            if (!virt) virt = 0x3000000;
            virt = (virt + 0xFFF) & ~0xFFFULL;

            for (u64 i = 0; i < fbpages; i++) {
                paging_map_page_proc(
                    g_ulime->hpr,
                    proc->pml4_phys,
                    virt + i * 4096,
                    fbphys + i * 4096,
                    USER_FLAGS | PTE_PCD
                );
            }
            proc->mmap_base = virt + fbpages * 4096;
            return virt;
        }
    }
    return (u64)MAP_FAILED;
}
u64 scall_munmap(ulime_proc_t *proc, u64 addr, u64 length, u64 arg3) {
    (void)proc; (void)arg3;
    if (!addr) return (u64)-1;

    u64 len   = (length + 0xFFF) & ~0xFFFULL;
    u64 pages = len / 4096;

    for (u64 i = 0; i < pages; i++) {
        paging_unmap_page(addr + i * 4096);
    }
    return 0;
}*/

u64 scall_waitpid(ulime_proc_t *proc, u64 pid, u64 arg2, u64 arg3)
{
    (void)arg2;
    (void)arg3;

    // find child
    ulime_proc_t *child = NULL;
    ulime_proc_t *p = g_ulime->ptr_proc_list;
    while (p) {
        if (p->pid == pid) { child = p; break; }
        p = p->next;
    }

    if (!child) return (u64)-1;

    // child already done
    if (child->state == PROC_ZOMBIE) return pid;

    // block parent until child exits
    if (blocked_count < MAX_BLOCKED) {
        blocked_entry_t *e = &blocked_list[blocked_count++];
        e->proc = proc;
        e->rip  = user_rcx;
        e->rsp  = user_rsp;
        e->r11  = user_r11;
        e->cr3  = user_cr3;
        e->rbx  = user_rbx;
        e->rbp  = user_rbp;
        e->r12  = user_r12;
        e->r13  = user_r13;
        e->r14  = user_r14;
        e->r15  = user_r15;
        e->waiting_for_pid = pid;
    }

    proc->state = PROC_BLOCKED;
    g_ulime->ptr_proc_curr = NULL;

    // find next ready task
    extern mt_t *mt;
    if (mt) {
        for (int i = 0; i < mt->task_count; i++) {
            mt_task_t *t = &mt->tasks[i];
            if (!t->valid || !t->proc) continue;
            if (t->proc->state == PROC_ZOMBIE || t->proc->state == PROC_BLOCKED) continue;
            mt->current_idx = i;
            t->proc->state = PROC_RUNNING;
            g_ulime->ptr_proc_curr = t->proc;
            if (t->kstack)
                gdt_set_kernel_stack(((u64)t->kstack + MT_KSTACK_SIZE) & ~0xFULL);
            printf(
            	"[SYSCALL] waitpid: '%s' waiting for pid=%llu, running '%s'\n",
                proc->name, pid, t->proc->name
            );
            if (t->user_ctx.saved) {
                resume_parent_sysret(
                    t->user_ctx.rip, t->user_ctx.rsp,
                    t->user_ctx.rflags, t->proc->pml4_phys,
                    t->user_ctx.rbx, t->user_ctx.rbp,
                    t->user_ctx.r12, t->user_ctx.r13,
                    t->user_ctx.r14, t->user_ctx.r15
                );
            } else {
                JumpToUserspace(t->proc);
            }
            __builtin_unreachable();
        }
    }

    printf("[SYSCALL] waitpid: no ready task, spinning\n");
    for (;;) __asm__ volatile("hlt");
    return 0;
}

u64 scall_sysinfo(ulime_proc_t *proc, u64 info_addr, u64 a1, u64 a2) {
	(void)a1;
	(void)a2;

	if (!is_valid_user_ptr_range(info_addr, sizeof(struct sysinfo_t))) return (u64) -1;

	struct sysinfo_t *info = (struct sysinfo_t *)info_addr;
	info->uptime = timer_get_uptime_seconds();

	return 0;
}

void _init_syscalls_table(ulime_t *ulime_ptr) {
    if (!ulime_ptr) return;

    g_ulime = ulime_ptr;
    memset(ulime_ptr->syscalls, 0, sizeof(ulime_ptr->syscalls));

    ulime_ptr->syscalls[READ]            = scall_read;
    ulime_ptr->syscalls[WRITE]           = scall_write;
    //ulime->syscalls[READ]   = scall_read;
    ulime_ptr->syscalls[OPEN]            = scall_open;
    ulime_ptr->syscalls[CLOSE]           = scall_close;
    ulime_ptr->syscalls[GETPID]          = scall_getpid;
    ulime_ptr->syscalls[BRK]             = scall_brk;
    ulime_ptr->syscalls[EXIT]            = scall_exit;
    ulime_ptr->syscalls[EXECVE]          = scall_execve;
    ulime_ptr->syscalls[GETDENTS]        = scall_getdents;
    ulime_ptr->syscalls[CHDIR]           = scall_chdir;
    ulime_ptr->syscalls[GETCWD]          = scall_getcwd;
    ulime_ptr->syscalls[IOCTL]           = scall_ioctl;
    ulime_ptr->syscalls[MQ_OPEN]         = scall_mq_open;
    ulime_ptr->syscalls[MQ_UNLINK]       = scall_mq_unlink;
    ulime_ptr->syscalls[MQ_SEND]         = scall_mq_send;
    ulime_ptr->syscalls[MQ_RECV]         = scall_mq_recv;
    //ulime_ptr->syscalls[SHM_DESTROY]       = scall_shm_destroy;
    ulime_ptr->syscalls[MMAP]            = scall_mmap;
    ulime_ptr->syscalls[MUNMAP]          = scall_munmap;
    ulime_ptr->syscalls[FORK]            = scall_fork;
    ulime_ptr->syscalls[WAITPID]         = scall_waitpid;

    // emex specific
    ulime_ptr->syscalls[EMXREBOOT]       = scall_reboot;
	ulime_ptr->syscalls[EMXSYSINFO]      = scall_sysinfo;

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
#include "ulime.h"
#include <klib/debug/serial.h>
#include <kernel/exceptions/panic.h>
#include <klib/memory/main.h>
#include <klib/string/string.h>
#include <kernel/mem/paging/paging.h>
#include <klib/graphics/theme.h>
#include <theme/doccr.h>

ulime_t *ulime_init(limine_hhdm_response_t *hpr, klime_t *klime, glime_t *glime, u64 uphys_start) {
    if (!klime || !glime) return NULL;

    ulime_t *ulime = (ulime_t *)(u64 *)ULIME_START;

    memset(ulime, 0, ULIME_META_SIZE);


    ulime->klime = klime;
    ulime->glime = glime;
    ulime->ptr_proc_list = NULL;
    ulime->ptr_proc_curr = NULL;
    ulime->ptr_thread = NULL;
    ulime->pid_next = 1;
    ulime->tid_next = 1;

    ulime->internal_pool_base = ULIME_START + sizeof(ulime_t);
    ulime->internal_pool_size = ULIME_META_SIZE - sizeof(ulime_t);
    ulime->internal_pool_used = 0;

    // USER address space management
    ulime->user_space_base = 0x40000000;  // User space starts at 1GB
    ulime->user_space_size = 0x00000000FFFFFFFF;  // Lower 4GB for users
    ulime->user_space_used = ulime->user_space_base;
    ulime->hpr = hpr;


    return ulime;
}

ulime_proc_t *ulime_proc_create(ulime_t *ulime, u8 *name, u64 entry_point) {
    // Allocate from ULife INTERNAL pool
    if (ulime->internal_pool_used + sizeof(ulime_proc_t) > ulime->internal_pool_size) {
        return NULL; // Out of internal memory
    }

    ulime_proc_t *proc = (ulime_proc_t*)(ulime->internal_pool_base + ulime->internal_pool_used);
    ulime->internal_pool_used += sizeof(ulime_proc_t);

    proc->ulime = ulime;

    // Initialize process
    memset(proc, 0, sizeof(ulime_proc_t));
    proc->pid = ulime->pid_next++;
    int i = 0;
    while (name[i] != '\0')
    {
        proc->name[i] = name[i];
        i++;
    }
    proc->name[i] = '\0';

    proc->state = PROC_CREATED;
    proc->entry_point = entry_point;

    // Allocate USER memory for this process
    proc->stack_base = ulime->user_space_used;
    proc->stack_size = 16 * 1024;  // 16KB stack
    ulime->user_space_used += proc->stack_size;

    proc->heap_base = ulime->user_space_used;
    proc->heap_size = 64 * 1024;   // 64KB initial heap
    ulime->user_space_used += proc->heap_size;

    // Add to process list
    proc->next = ulime->ptr_proc_list;
    if (ulime->ptr_proc_list) ulime->ptr_proc_list->prev = proc;
    ulime->ptr_proc_list = proc;

    BOOTUP_PRINTF("Created process %s:\n", proc->name);
    BOOTUP_PRINTF("  Stack: %lX (%lu KB)\n", proc->stack_base, proc->stack_size / 1024);
    BOOTUP_PRINTF("  Heap:  %lX (%lu KB)\n", proc->heap_base, proc->heap_size / 1024);

    return proc;
}


int ulime_proc_kill(ulime_t *ulime, u64 pid) {
    ulime_proc_t *proc = ulime->ptr_proc_list;
    while (proc) {
        if (proc->pid == pid) {
            BOOTUP_PRINTF("Killing process %s (PID %lu)\n", proc->name, pid);
            proc->state = PROC_ZOMBIE;
            return 0;
        }
        proc = proc->next;
    }
    BOOTUP_PRINTF("Process with PID %lu not found\n", pid);
    return 1;
}

int ulime_proc_mmap(ulime_t *ulime, ulime_proc_t *proc) {
    u64 phys_stack_len = proc->stack_size / PAGE_SIZE;
    u64 phys_stack_pos = physmem_alloc_to(phys_stack_len);

    if (!phys_stack_pos) {
        BOOTUP_PRINTF("Erorr: proc stack pos is not allocated\n");
        return 1;
    }

    u64 phys_heap_len  = proc->heap_size / PAGE_SIZE;
    u64 phys_heap_pos  = physmem_alloc_to(phys_heap_len);
    if (!phys_heap_pos) {
        BOOTUP_PRINTF("Erorr: proc heap pos is not allocated\n");
        return 1;
    }

    u64 kernel_start = 0x204000; // kernel physical address @MaybeBug
    u64 kernel_end = kernel_start + HEAP_SIZE;


    map_region(ulime->hpr, phys_stack_pos, proc->stack_base, proc->stack_size);
    map_region(ulime->hpr, phys_heap_pos, proc->heap_base, proc->heap_size);

    BOOTUP_PRINTF("Mapped process %s memory:\n", proc->name);
    BOOTUP_PRINTF("  Stack: 0x%lX → 0x%lX\n", proc->stack_base, phys_stack_pos);
    BOOTUP_PRINTF("  Heap:  0x%lX → 0x%lX\n", proc->heap_base, phys_heap_pos);

    proc->phys_stack = phys_stack_pos;
    proc->phys_heap = phys_heap_pos;

    return 0;
}

void ulime_proc_test_mem(ulime_proc_t *proc) {
    BOOTUP_PRINTF("Testing process memory access...\n");

    // Test stack access
    u32 *stack_test = (u32*)proc->stack_base;
    *stack_test = 0x12345678;
    BOOTUP_PRINTF("  Wrote 0x%X to stack at 0x%lX\n", *stack_test, proc->stack_base);

    // Test heap access
    u32 *heap_test = (u32*)proc->heap_base;
    *heap_test = 0xABCDEF00;
    BOOTUP_PRINTF("  Wrote 0x%X to heap at 0x%lX\n", *heap_test, proc->heap_base);

    BOOTUP_PRINTF("Process memory access test passed!\n");
}

void ulime_schedule(ulime_t *ulime) {
    if (!ulime->ptr_proc_list) return;

    // Simple round-robin scheduling
    if (!ulime->ptr_proc_curr) {
        // No current process, pick first one
        ulime->ptr_proc_curr = ulime->ptr_proc_list;
    } else {
        // Move to next process
        ulime->ptr_proc_curr = ulime->ptr_proc_curr->next;
        if (!ulime->ptr_proc_curr) {
            ulime->ptr_proc_curr = ulime->ptr_proc_list;
        }
    }

    // Mark as running
    if (ulime->ptr_proc_curr->state == PROC_READY) {
        ulime->ptr_proc_curr->state = PROC_RUNNING;
        BOOTUP_PRINTF("Scheduled process: %s (PID %lu)\n",
               ulime->ptr_proc_curr->name, ulime->ptr_proc_curr->pid);
    }
}

void ulime_proc_list(ulime_t *ulime) {
    BOOTUP_PRINTF("Process List:\n");
    BOOTUP_PRINTF("PID\tState\tName\t\tEntry Point\n");
    BOOTUP_PRINTF("---\t-----\t----\t\t-----------\n");

    ulime_proc_t *current = ulime->ptr_proc_list;
    while (current) {
        const char *state_str = "UNKNOWN";
        switch (current->state) {
            case PROC_CREATED: state_str = "CREATED"; break;
            case PROC_READY:   state_str = "READY"; break;
            case PROC_RUNNING: state_str = "RUNNING"; break;
            case PROC_BLOCKED: state_str = "BLOCKED"; break;
            case PROC_ZOMBIE:  state_str = "ZOMBIE"; break;
        }

        BOOTUP_PRINTF("%lu\t%s\t%s\t\t0x%lX\n",
               current->pid, state_str, current->name, current->entry_point);
        current = current->next;
    }
}

//dont works
int ulime_load_program(ulime_proc_t *proc, u8 *code, u64 code_size) {
    if (code_size > proc->heap_size) {
        BOOTUP_PRINTF("Program too large for process heap\n");
        return 1;
    }

    // Copy program code to process heap
    memcpy((void*)proc->heap_base, code, code_size);

    // Set entry point to start of heap
    proc->entry_point = proc->heap_base;

    BOOTUP_PRINTF("Loaded program (%lu bytes) into process %s at 0x%lX\n",
                    code_size, proc->name, proc->entry_point);
    return 0;
}

u64 sys_exit(ulime_proc_t *proc, u64 exit_code, u64 arg2, u64 arg3) {
    BOOTUP_PRINTF("Process %s (PID %lu) exiting with code %lu\n",
           proc->name, proc->pid, exit_code);
    proc->state = PROC_ZOMBIE;
    return 0;
}

u64 sys_write(ulime_proc_t *proc, u64 fd, u64 buf, u64 count) {
    u8 *str = (u8 *)buf;
    for (u64 i = 0; i < count; i++) {
        count ++;
    }
    gsession_put_at_string_dummy(
        proc->ulime->glime->workspaces[0]->sessions[0],
        str,
        0, 0,
        0x00FFFFFF
    );
    return count;
}

void ulime_init_syscalls(ulime_t *ulime) {
    memset(ulime->syscalls, 0, sizeof(ulime->syscalls));

    ulime->syscalls[SYS_EXIT]   = sys_exit;
    ulime->syscalls[SYS_WRITE]  = sys_write;
}

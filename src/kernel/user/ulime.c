#include "ulime.h"
#include <kernel/communication/serial.h>
#include <kernel/arch/x86_64/exceptions/panic.h>
#include <memory/main.h>
#include <string/string.h>
#include <kernel/mem/paging/paging.h>
#include <kernel/graph/theme.h>
#include <theme/doccr.h>

// ulime == (U)ser (LI)fe ti(ME)

ulime_t *ulime_init(limine_hhdm_response_t *hpr, klime_t *klime, void *glime, u64 uphys_start) {
    (void)uphys_start;

    if (!klime) return NULL;

    ulime_t *ulime = (ulime_t *)(u64 *)ULIME_START;
    memset(ulime, 0, ULIME_META_SIZE);

    ulime->klime = klime;

    #if ENABLE_GLIME
        ulime->glime = (glime_t *)glime;
    #else
        ulime->glime = NULL;
        (void)glime;
    #endif

    ulime->ptr_proc_list = NULL;
    ulime->ptr_proc_curr = NULL;
    ulime->ptr_thread = NULL;
    ulime->pid_next = 1;
    ulime->tid_next = 1;

    ulime->internal_pool_base = ULIME_START + sizeof(ulime_t);
    ulime->internal_pool_size = ULIME_META_SIZE - sizeof(ulime_t);
    ulime->internal_pool_used = 0;

    // user address space
    ulime->user_space_base = 0x40000000;  // 1GB
    ulime->user_space_size = 0x40000000;  // 1GB range
    ulime->user_space_used = ulime->user_space_base;
    ulime->hpr = hpr;

    log("[ULIME]","initialized at user_space_base=", d);
    char buf[32];
    //str_copy(buf, "user_space_base=");
    str_append_uint(buf, ulime->user_space_base);
    log("", buf, d);
    BOOTUP_PRINT("\n", white());

    return ulime;
}

ulime_proc_t *ulime_proc_create(ulime_t *ulime, u8 *name, u64 entry_point) {
    // Allocate from ULife INTERNAL pool
    if (ulime->internal_pool_used + sizeof(ulime_proc_t) > ulime->internal_pool_size) {
        printf("[ULIME] ERROR: Out of internal memory\n");
        return NULL; // Out of internal memory
    }

    ulime_proc_t *proc = (ulime_proc_t*)(ulime->internal_pool_base + ulime->internal_pool_used);
    ulime->internal_pool_used += sizeof(ulime_proc_t);

    //proc->ulime = ulime;

    // Initialize process
    memset(proc, 0, sizeof(ulime_proc_t));
    proc->ulime = ulime;
    proc->pid = ulime->pid_next++;

    int i = 0;
    while (name[i] != '\0' && i < 63) {
        proc->name[i] = name[i];
        i++;
    }

    proc->name[i] = '\0';

    proc->state = PROC_CREATED;
    proc->entry_point = entry_point;

    proc->heap_size = 256 * 1024;
    proc->heap_base = (ulime->user_space_used + 0xFFF) & ~0xFFF;
    ulime->user_space_used = proc->heap_base + proc->heap_size;
    proc->stack_size = 64 * 1024;
    proc->stack_base = (ulime->user_space_used + 0xFFF) & ~0xFFF;
    ulime->user_space_used = proc->stack_base + proc->stack_size;

    printf("[ULIME] Process '%s' created:\n", proc->name);
    printf("  PID: %llu\n", (unsigned long long)proc->pid);
    printf("  Stack: 0x%llx - 0x%llx (%llu KB)\n",
           (unsigned long long)proc->stack_base,
           (unsigned long long)(proc->stack_base + proc->stack_size),
           (unsigned long long)(proc->stack_size / 1024));
    printf("  Heap:  0x%llx - 0x%llx (%llu KB)\n",
           (unsigned long long)proc->heap_base,
           (unsigned long long)(proc->heap_base + proc->heap_size),
           (unsigned long long)(proc->heap_size / 1024));

    // Add to process list
    proc->next = ulime->ptr_proc_list;
    if (ulime->ptr_proc_list) ulime->ptr_proc_list->prev = proc;
    ulime->ptr_proc_list = proc;

    return proc;
}

int ulime_proc_mmap(ulime_t *ulime, ulime_proc_t *proc) {
    printf("[ULIME] Mapping memory for process '%s'...\n", proc->name);

    u64 stack_pages = (proc->stack_size + 0xFFF) / PAGE_SIZE;
    u64 phys_stack = physmem_alloc_to(stack_pages);
    if (!phys_stack) {
        printf("[ULIME] ERROR: Failed to allocate physical stack\n");
        return 1;
    }
    printf("  Stack phys: 0x%llx (%llu pages)\n",
           (unsigned long long)phys_stack, (unsigned long long)stack_pages);

    u64 heap_pages = (proc->heap_size + 0xFFF) / PAGE_SIZE;
    u64 phys_heap = physmem_alloc_to(heap_pages);
    if (!phys_heap) {
        printf("[ULIME] ERROR: Failed to allocate physical heap\n");
        physmem_free_to(phys_stack, stack_pages);
        return 1;
    }
    printf("  Heap phys:  0x%llx (%llu pages)\n",
           (unsigned long long)phys_heap, (unsigned long long)heap_pages);

    u64 pml4_phys = paging_create_proc_pml4(ulime->hpr);
    proc->pml4_phys = pml4_phys;
    printf("  PML4 phys:  0x%llx\n", (unsigned long long)pml4_phys);

    u64 stack_flags = PTE_PRESENT | PTE_WRITABLE | PTE_USER | PTE_NO_EXEC;
    u64 heap_flags  = PTE_PRESENT | PTE_WRITABLE | PTE_USER;

    // map heap/code into the process's own PML4
    for (u64 i = 0; i < heap_pages; i++) {
        u64 virt = proc->heap_base + (i * PAGE_SIZE);
        u64 phys = phys_heap + (i * PAGE_SIZE);
        paging_map_page_proc(ulime->hpr, pml4_phys, virt, phys, heap_flags);
    }

    // map stack into the processes own PML4
    for (u64 i = 0; i < stack_pages; i++) {
        u64 virt = proc->stack_base + (i * PAGE_SIZE);
        u64 phys = phys_stack + (i * PAGE_SIZE);
        paging_map_page_proc(ulime->hpr, pml4_phys, virt, phys, stack_flags);
    }

    // clear memory via HHDM
    void *heap_clear  = (void *)(phys_heap  + ulime->hpr->offset);
    void *stack_clear = (void *)(phys_stack + ulime->hpr->offset);
    memset(heap_clear,  0, proc->heap_size);
    memset(stack_clear, 0, proc->stack_size);

    proc->phys_stack = phys_stack;
    proc->phys_heap  = phys_heap;
    proc->state = PROC_READY;

    printf("[ULIME] memory mapped successfully\n");
    return 0;
}

int ulime_proc_kill(ulime_t *ulime, u64 pid) {
    ulime_proc_t *proc = ulime->ptr_proc_list;
    while (proc) {
        if (proc->pid == pid) {
            proc->state = PROC_ZOMBIE;
            return 0;
        }
        proc = proc->next;
    }
    return 1;
}

void ulime_schedule(ulime_t *ulime) {
    if (!ulime->ptr_proc_list) return;

    // simple round-robin scheduling
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
    }
}

static void append_spaces(char *buf, int count){
    for (int i = 0; i < count; i++)
        str_append(buf, " ");
}

void ulime_proc_list(ulime_t *ulime)
{
    char line[256];
    char numbuf[32];

    BOOTUP_PRINT("Process List:\n", white());
    BOOTUP_PRINT("PID   State       Name            Entry Point\n", white());
    BOOTUP_PRINT("----  ----------  --------------  -----------\n", white());

    ulime_proc_t *current = ulime->ptr_proc_list;
    while (current)
    {
        const char *state_str = "UNKNOWN";
        switch (current->state)
        {
            case PROC_CREATED: state_str = "CREATED"; break;
            case PROC_READY:   state_str = "READY"; break;
            case PROC_RUNNING: state_str = "RUNNING"; break;
            case PROC_BLOCKED: state_str = "BLOCKED"; break;
            case PROC_ZOMBIE:  state_str = "ZOMBIE"; break;
        }

        line[0] = 0;

        str_from_int(numbuf, (int)current->pid);
        str_append(line, numbuf);
        append_spaces(line, PIDSPACE - str_len(numbuf) + 2);

        str_append(line, state_str);
        append_spaces(line, STATSPACE - str_len(state_str) + 2);

        str_append(line, (char*)current->name);
        append_spaces(line, NAMESPACE - str_len((char*)current->name) + 2);

        // we need str_from_hex because otherwise it would print hex-prefix + decimal number
        str_append(line, "0x");
        str_from_hex(numbuf, current->entry_point);
        str_append(line, numbuf);

        str_append(line, "\n");

        BOOTUP_PRINT(line, white());

        current = current->next;
    }
}

int ulime_load_program(ulime_proc_t *proc, u8 *code, u64 code_size) {
    if (code_size > proc->heap_size) {
        BOOTUP_PRINTF("Program too large for process heap\n");
        return 1;
    }

    memcpy((void*)proc->heap_base, code, code_size);
    proc->entry_point = proc->heap_base;

    BOOTUP_PRINTF("Loaded program (%lu bytes) into process %s at 0x%lX\n",
           code_size, proc->name, proc->entry_point);
    return 0;
}

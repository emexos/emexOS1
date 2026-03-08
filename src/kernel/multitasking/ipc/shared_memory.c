#include "ipc.h"
#include <memory/main.h>
#include <kernel/communication/serial.h>
#include <kernel/mem/phys/physmem.h>

static ipc_shm_t shm_table[IPC_MAX_SHM];
static u64 shm_counter = 1;

void ipc_shm_init(void) {
    for (int i = 0; i < IPC_MAX_SHM; i++) {
        shm_table[i].used = 0;
        shm_table[i].id = 0;
        shm_table[i].ref_count = 0;
    }
}

static ipc_shm_t *shm_find(u64 id) {
    for (int i = 0; i < IPC_MAX_SHM; i++) {
        if (shm_table[i].used && shm_table[i].id == id)
            return &shm_table[i];
    }
    return NULL;
}

static ipc_shm_t *shm_alloc_slot(void) {
    for (int i = 0; i < IPC_MAX_SHM; i++) {
        if (!shm_table[i].used)
            return &shm_table[i];
    }
    return NULL;
}

static void shm_free_phys(ipc_shm_t *shm)
{
    for (u32 i = 0; i < shm->page_count; i++) {
        if (shm->phys_pages[i]) {
            physmem_free_to(shm->phys_pages[i], 1);
            shm->phys_pages[i] = 0;
        }
    }
    shm->used = 0;
    shm->ref_count = 0;
}

u64 shm_create(ulime_t *ulime, u32 pages)
{
    (void)ulime;
    if (pages == 0 || pages > IPC_MAX_SHM_PAGES) return IPC_SHM_INVALID;

    ipc_shm_t *shm = shm_alloc_slot();
    if (!shm) return IPC_SHM_INVALID;

    for (u32 i = 0; i < pages; i++) {
        u64 phys = physmem_alloc_to(1);
        if (!phys) {
            for (u32 j = 0; j < i; j++)
                physmem_free_to(shm->phys_pages[j], 1);
            return IPC_SHM_INVALID;
        }
        shm->phys_pages[i] = phys;
    }

    shm->id = shm_counter++;
    shm->page_count = pages;
    shm->ref_count = 0;
    shm->used = 1;

    for (int i = 0; i < IPC_MAX_PROCS; i++)
        shm->virt_per_proc[i] = 0;

    printf("[SHM] created id=%llu pages=%u\n", (unsigned long long)shm->id, pages);
    return shm->id;
}

void *shm_attach(ulime_t *ulime, ulime_proc_t *proc, u64 id)
{
    ipc_shm_t *shm = shm_find(id);
    if (!shm) return NULL;

    u64 pid = proc->pid;
    if (pid >= IPC_MAX_PROCS) return NULL;

    if (shm->virt_per_proc[pid] != 0)
        return (void *)shm->virt_per_proc[pid];

    u64 virt = (ulime->user_space_used + 0xFFF) & ~0xFFFULL;
    ulime->user_space_used = virt + (u64)shm->page_count * PAGE_SIZE;

    u64 flags = PTE_PRESENT | PTE_WRITABLE | PTE_USER;
    for (u32 i = 0; i < shm->page_count; i++) {
        paging_map_page_proc(
        	ulime->hpr,
            proc->pml4_phys,
            virt + (u64)i * PAGE_SIZE,
            shm->phys_pages[i],
            flags
        );
    }

    shm->virt_per_proc[pid] = virt;
    shm->ref_count++;

    printf("[SHM] attached id=%llu to pid=%llu virt=0x%llx\n", (unsigned long long)id, (unsigned long long)pid, (unsigned long long)virt);
    return (void *)virt;
}

void shm_detach(ulime_t *ulime, ulime_proc_t *proc, u64 id) {
    ipc_shm_t *shm = shm_find(id);
    if (!shm) return;

    u64 pid = proc->pid;
    if (pid >= IPC_MAX_PROCS) return;
    if (shm->virt_per_proc[pid] == 0) return;

    u64 virt = shm->virt_per_proc[pid];
    for (u32 i = 0; i < shm->page_count; i++) {
        u64 va = virt + (u64)i * PAGE_SIZE;
        paging_map_page_proc(ulime->hpr, proc->pml4_phys, va, 0, 0);
        __asm__ volatile("invlpg (%0)" :: "r"(va) : "memory");
    }

    shm->virt_per_proc[pid] = 0;
    shm->ref_count--;

    printf("[SHM] detached id=%llu from pid=%llu\n",(unsigned long long)id, (unsigned long long)pid);

    if (shm->ref_count <= 0)
        shm_free_phys(shm);
}

void shm_detach_all(ulime_t *ulime, ulime_proc_t *proc) {
    u64 pid = proc->pid;
    if (pid >= IPC_MAX_PROCS) return;

    for (int i = 0; i < IPC_MAX_SHM; i++) {
        if (shm_table[i].used && shm_table[i].virt_per_proc[pid] != 0)
            shm_detach(ulime, proc, shm_table[i].id);
    }
}
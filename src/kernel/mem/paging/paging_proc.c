#include "paging.h"
#include <limine/limine.h>
#include <kernel/mem/phys/physmem.h>
#include <kernel/arch/x86_64/exceptions/panic.h>
#include <memory/main.h>
#include <string/string.h>
#include <kernel/communication/serial.h>

u64 paging_create_proc_pml4(limine_hhdm_response_t *hpr)
{
    u64 pml4_phys = physmem_alloc_to(1);
    if (!pml4_phys) panic("[PAGING] failed to allocate proc PML4");

    page_table_t *pml4 = (page_table_t *)(pml4_phys + hpr->offset);
    memset(pml4, 0, PAGE_SIZE);

    // share kernel upper half so syscalls/kernel code work in every process
    for (int i = 256; i < 512; i++) {
        pml4->entries[i] = kernel_pml4->entries[i];
    }

    return pml4_phys;
}

void paging_map_page_proc(
    limine_hhdm_response_t *hpr,
    u64 pml4_phys,
    u64 virtual_addr,
    u64 physical_addr,
    u64 flags)
{
    page_table_t *pml4 = (page_table_t *)(pml4_phys + hpr->offset);

    u64 pml4_index = (virtual_addr >> 39) & 0x1FF;
    u64 pdp_index  = (virtual_addr >> 30) & 0x1FF;
    u64 pd_index   = (virtual_addr >> 21) & 0x1FF;
    u64 pt_index   = (virtual_addr >> 12) & 0x1FF;

    u64 upper_flags = PTE_PRESENT | PTE_WRITABLE;
    if (flags & PTE_USER) upper_flags |= PTE_USER;

    page_table_t *pdpt = NULL;
    if (!(pml4->entries[pml4_index] & PTE_PRESENT)) {
        u64 pdpt_phys = physmem_alloc_to(1);
        if (!pdpt_phys) panic("[PAGING] failed to allocate PDPT for proc");
        pml4->entries[pml4_index] = (pdpt_phys & 0x000FFFFFFFFFF000) | upper_flags;
        pdpt = (page_table_t *)(pdpt_phys + hpr->offset);
        memset(pdpt, 0, PAGE_SIZE);
    } else {
        u64 pdpt_phys = pml4->entries[pml4_index] & 0x000FFFFFFFFFF000;
        pdpt = (page_table_t *)(pdpt_phys + hpr->offset);
        pml4->entries[pml4_index] |= upper_flags;
    }

    page_table_t *pd = NULL;
    if (!(pdpt->entries[pdp_index] & PTE_PRESENT)) {
        u64 pd_phys = physmem_alloc_to(1);
        if (!pd_phys) panic("[PAGING] failed to allocate PD for proc");
        pdpt->entries[pdp_index] = (pd_phys & 0x000FFFFFFFFFF000) | upper_flags;
        pd = (page_table_t *)(pd_phys + hpr->offset);
        memset(pd, 0, PAGE_SIZE);
    } else {
        u64 pd_phys = pdpt->entries[pdp_index] & 0x000FFFFFFFFFF000;
        pd = (page_table_t *)(pd_phys + hpr->offset);
        pdpt->entries[pdp_index] |= upper_flags;
    }

    page_table_t *pt = NULL;
    if (!(pd->entries[pd_index] & PTE_PRESENT)) {
        u64 pt_phys = physmem_alloc_to(1);
        if (!pt_phys) panic("[PAGING] failed to allocate PT for proc");
        pd->entries[pd_index] = (pt_phys & 0x000FFFFFFFFFF000) | upper_flags;
        pt = (page_table_t *)(pt_phys + hpr->offset);
        memset(pt, 0, PAGE_SIZE);
    } else {
        u64 pt_phys = pd->entries[pd_index] & 0x000FFFFFFFFFF000;
        pt = (page_table_t *)(pt_phys + hpr->offset);
        pd->entries[pd_index] |= upper_flags;
    }

    pt->entries[pt_index] = (physical_addr & 0x000FFFFFFFFFF000) | flags;

	__asm__ volatile("invlpg (%0)" : : "r"(virtual_addr) : "memory");
}
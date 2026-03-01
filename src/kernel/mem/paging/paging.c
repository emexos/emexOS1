#include "paging.h"
#include <limine/limine.h>

#include "../mem.h"
#include "../phys/physmem.h"
#include <kernel/arch/x86_64/exceptions/panic.h>
#include <memory/main.h>

#include <string/string.h>
#include <theme/stdclrs.h>
#include <kernel/communication/serial.h>

extern u8 _kernel_start[];
extern u8 _kernel_end[];

page_table_t *kernel_pml4 = NULL;

inline void* phys_to_virt(limine_hhdm_response_t *hpr, u64 phys_addr) {
    return (void*)(phys_addr + hpr->offset);
}

// Convert kernel virtual address to physical address using HHDM
inline u64 virt_to_phys(limine_hhdm_response_t *hpr, void* virt_addr) {
    return (u64)virt_addr - hpr->offset;
}

/// Summary
/// 2025/11/17 tsaraki
/// so paging of a 4096 bytes
void paging_map_page(
    limine_hhdm_response_t *hpr,
    u64 virtual_addr,
    u64 physical_addr,
    u64 flags
) {

    u64 pml4_index = (virtual_addr >> 39) & 0x1FF;
    u64 pdp_index  = (virtual_addr >> 30) & 0x1FF;
    u64 pd_index   = (virtual_addr >> 21) & 0x1FF;
    u64 pt_index   = (virtual_addr >> 12) & 0x1FF;

    u64 upper_flags = PTE_PRESENT | PTE_WRITABLE;
    if (flags & PTE_USER)
        upper_flags |= PTE_USER;

    page_table_t* pdpt = NULL;
    if (!(kernel_pml4->entries[pml4_index] & PTE_PRESENT)) {
        u64 pdpt_phys = physmem_alloc_to(1);
        if (!pdpt_phys) panic("Could not allocate PDPT!");

        kernel_pml4->entries[pml4_index] =
            (pdpt_phys & 0x000FFFFFFFFFF000) | upper_flags;

        pdpt = (page_table_t*)(pdpt_phys + hpr->offset);
        memset(pdpt, 0, PAGE_SIZE);
    } else {
        u64 pdpt_phys = kernel_pml4->entries[pml4_index] & 0x000FFFFFFFFFF000;
        pdpt = (page_table_t*)(pdpt_phys + hpr->offset);

        kernel_pml4->entries[pml4_index] |= upper_flags;
    }

    page_table_t* pd = NULL;
    if (!(pdpt->entries[pdp_index] & PTE_PRESENT)) {
        u64 pd_phys = physmem_alloc_to(1);
        if (!pd_phys) panic("Could not allocate PD!");

        pdpt->entries[pdp_index] =
            (pd_phys & 0x000FFFFFFFFFF000) | upper_flags;

        pd = (page_table_t*)(pd_phys + hpr->offset);
        memset(pd, 0, PAGE_SIZE);
    } else {
        u64 pd_phys = pdpt->entries[pdp_index] & 0x000FFFFFFFFFF000;
        pd = (page_table_t*)(pd_phys + hpr->offset);

        pdpt->entries[pdp_index] |= upper_flags;
    }

    page_table_t* pt = NULL;
    if (!(pd->entries[pd_index] & PTE_PRESENT)) {
        u64 pt_phys = physmem_alloc_to(1);
        if (!pt_phys) panic("Could not allocate PT!");

        pd->entries[pd_index] =
            (pt_phys & 0x000FFFFFFFFFF000) | upper_flags;

        pt = (page_table_t*)(pt_phys + hpr->offset);
        memset(pt, 0, PAGE_SIZE);
    } else {
        u64 pt_phys = pd->entries[pd_index] & 0x000FFFFFFFFFF000;
        pt = (page_table_t*)(pt_phys + hpr->offset);

        pd->entries[pd_index] |= upper_flags;
    }

    // Map the page
    pt->entries[pt_index] =
        ((physical_addr & 0x000FFFFFFFFFF000) | flags) & ~PTE_NX;

    // Invalidate TLB entry
    __asm__ volatile("invlpg (%0)" : : "r" (virtual_addr) : "memory");
}


void paging_init(limine_hhdm_response_t *hpr) {
    u64 current_cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r" (current_cr3));

    kernel_pml4 = (page_table_t*)((current_cr3 & 0x000FFFFFFFFFF000) + hpr->offset);
}

u64 map_region_alloc(limine_hhdm_response_t *hpr, u64 virt, u64 size) {
    u64 phys_frames = size / PAGE_SIZE;
    u64 phys = physmem_alloc_to(phys_frames);
    if  (!phys) panic("ERROR: Could not allocate physmem in lime request");

    //map_region(hpr, phys, virt, size);
    map_region(hpr, phys, virt, size, PTE_PRESENT | PTE_WRITABLE | PTE_USER);

    return phys;
}

void map_region(limine_hhdm_response_t *hpr, u64 phys, u64 virt, u64 size, u64 flags) {
    u64 frames = size / PAGE_SIZE;

    for (u64 i = 0; i < frames; i++) {
        u64 virt_to_page = virt + (i * PAGE_SIZE);
        u64 phys_to_page = phys + (i * PAGE_SIZE);

        paging_map_page(hpr, virt_to_page, phys_to_page, flags);
    }

    memset((void*)(phys + hpr->offset), 0, size);
}

int is_page_mapped(limine_hhdm_response_t *hpr, u64 virtual_addr) {
    u64 pml4_index = (virtual_addr >> 39) & 0x1FF;
    u64 pdp_index  = (virtual_addr >> 30) & 0x1FF;
    u64 pd_index   = (virtual_addr >> 21) & 0x1FF;
    u64 pt_index   = (virtual_addr >> 12) & 0x1FF;

    if (!(kernel_pml4->entries[pml4_index] & PTE_PRESENT)) return 0;

    u64 pdpt_phys = kernel_pml4->entries[pml4_index] & 0x000FFFFFFFFFF000;
    page_table_t* pdpt = (page_table_t*)(pdpt_phys + hpr->offset);

    if (!(pdpt->entries[pdp_index] & PTE_PRESENT)) return 0;

    u64 pd_phys = pdpt->entries[pdp_index] & 0x000FFFFFFFFFF000;
    page_table_t* pd = (page_table_t*)(pd_phys + hpr->offset);

    if (!(pd->entries[pd_index] & PTE_PRESENT)) return 0;

    u64 pt_phys = pd->entries[pd_index] & 0x000FFFFFFFFFF000;
    page_table_t* pt = (page_table_t*)(pt_phys + hpr->offset);

    return (pt->entries[pt_index] & PTE_PRESENT) != 0;
}

void map_ulime_region(limine_hhdm_response_t *hpr, u64 phys_start, u64 size) {
    if (ULIME_START % PAGE_SIZE != 0) {
        panic("ULIME_START not page aligned!");
    }

    if (phys_start % PAGE_SIZE != 0) {
        panic("ULime physical start not page aligned!");
    }

    u64 pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;

    for (u64 i = 0; i < pages; i++) {
        u64 virt_addr = ULIME_START + (i * PAGE_SIZE);
        u64 phys_addr = phys_start + (i * PAGE_SIZE);

        if (!is_page_mapped(hpr, virt_addr)) {
            paging_map_page(hpr, virt_addr, phys_addr, PTE_PRESENT | PTE_WRITABLE);
        }
    }
}
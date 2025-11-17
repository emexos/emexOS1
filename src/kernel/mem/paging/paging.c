#include "paging.h"
#include <limine.h>

#include "kheap.h"
#include <kernel/exceptions/panic.h>
#include <klib/memory/main.h>

#include "string.h"
#include "print.h"
#include "stdclrs.h"
#include <klib/debug/serial.h>

extern u8 _kernel_start[];
extern u8 _kernel_end[];

static page_table_t *kernel_pml4 = NULL;


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
    
    // Get or create PDPT
    page_table_t* pdpt = NULL;
    if (!(kernel_pml4->entries[pml4_index] & PTE_PRESENT)) {
        u64 pdpt_phys = physmem_alloc_to(1);
        if (!pdpt_phys) panic("Could not allocate PDPT!");
        
        kernel_pml4->entries[pml4_index] = (pdpt_phys & 0x000FFFFFFFFFF000) | PTE_PRESENT | PTE_WRITABLE;
        
        pdpt = (page_table_t*)(pdpt_phys + hpr->offset);
        memset(pdpt, 0, PAGE_SIZE);
    } else {
        u64 pdpt_phys = kernel_pml4->entries[pml4_index] & 0x000FFFFFFFFFF000;
        pdpt = (page_table_t*)(pdpt_phys + hpr->offset);
    }
    
    // Get or create PD
    page_table_t* pd = NULL;
    if (!(pdpt->entries[pdp_index] & PTE_PRESENT)) {
        u64 pd_phys = physmem_alloc_to(1);
        if (!pd_phys) panic("Could not allocate PD!");
        
        pdpt->entries[pdp_index] = (pd_phys & 0x000FFFFFFFFFF000) | PTE_PRESENT | PTE_WRITABLE;
        
        pd = (page_table_t*)(pd_phys + hpr->offset);
        memset(pd, 0, PAGE_SIZE);
    } else {
        u64 pd_phys = pdpt->entries[pdp_index] & 0x000FFFFFFFFFF000;
        pd = (page_table_t*)(pd_phys + hpr->offset);
    }
    
    // Get or create PT
    page_table_t* pt = NULL;
    if (!(pd->entries[pd_index] & PTE_PRESENT)) {
        u64 pt_phys = physmem_alloc_to(1);
        if (!pt_phys) panic("Could not allocate PT!");
        
        pd->entries[pd_index] = (pt_phys & 0x000FFFFFFFFFF000) | PTE_PRESENT | PTE_WRITABLE;
        
        pt = (page_table_t*)(pt_phys + hpr->offset);
        memset(pt, 0, PAGE_SIZE);
    } else {
        u64 pt_phys = pd->entries[pd_index] & 0x000FFFFFFFFFF000;
        pt = (page_table_t*)(pt_phys + hpr->offset);
    }
    
    // Map the page
    pt->entries[pt_index] = (physical_addr & 0x000FFFFFFFFFF000) | flags;
    
    // Invalidate TLB entry
    asm volatile("invlpg (%0)" : : "r" (virtual_addr) : "memory");
}

/// Summary
/// 2025/11/17 tsaraki
/// limine already done paging
/// just paging for kernel heap
/// @Question: paging for drivers, io, etc?
void paging_init(limine_hhdm_response_t *hpr) {
    u64 current_cr3;
    asm volatile("mov %%cr3, %0" : "=r" (current_cr3));
    
    kernel_pml4 = (page_table_t*)((current_cr3 & 0x000FFFFFFFFFF000) + hpr->offset);

    u64 heap_frames_len = (HEAP_SIZE / PAGE_SIZE);

    u64 phys = physmem_alloc_to(heap_frames_len);
    if  (!phys) panic("ERROR: Could not allocate physmem for heap");
    
    // paging for the kernel heap
    for (u64 i = 0; i < heap_frames_len; i++) {
        u64 virt = HEAP_START + (i * PAGE_SIZE);

        paging_map_page(hpr, virt, phys + i, PTE_PRESENT | PTE_WRITABLE);
    }

    // there should be paging of smth else
    // if it need to be done while kernel initializing
    
    return;
}



#ifndef PAGGING_H
#define PAGGING_H

#include <types.h>
#include "../phys/physmem.h"

#define PTE_PRESENT    (1ULL << 0)
#define PTE_WRITABLE   (1ULL << 1)
#define PTE_USER       (1ULL << 2)
#define PTE_PWT        (1ULL << 3)
#define PTE_PCD        (1ULL << 4)
#define PTE_ACCESSED   (1ULL << 5)
#define PTE_DIRTY      (1ULL << 6)
#define PTE_HUGE       (1ULL << 7)
#define PTE_GLOBAL     (1ULL << 8)
#define PTE_NO_EXEC    (1ULL << 63)

typedef struct {
    u64 entries[512];
} page_table_t;

void paging_init(limine_hhdm_response_t *hpr);
u64* paging_get_physical_address(u64 virtual_addr);
void paging_map_page(limine_hhdm_response_t *hpr, u64 virtual_addr, u64 physical_addr, u64 flags);
void paging_unmap_page(u64 virtual_addr);

#endif

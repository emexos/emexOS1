#ifndef PHYSMEM_H
#define PHYSMEM_H

#include <types.h>
#include "../mem.h"

typedef struct physmem_pageframe {
    u32 rc;
    u32 flags;
} physmem_pageframe_t;

u64 physmem_free_get(void);
u64 physmem_alloc_to(u64 count);
void physmem_free_to(u64 physmem_addr, u64 count);
void physmem_init(limine_memmap_response_t *mpr, limine_hhdm_response_t *hpr);

#endif

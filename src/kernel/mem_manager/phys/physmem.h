#ifndef PHYSMEM_H
#define PHYSMEM_H

#include <types.h>

#define PAGE_SIZE 4096
#define PAGES_PER_BLOCK 8

void physmem_init(u64 mem_size);
u64 physmem_alloc_page(void);
u64 physmem_alloc_pages(u32 n);
void physmem_free_page(u64 addr);
void physmem_get_free_pages(u64 addr, u32 n);

u64 physmem_get_free(void);
u64 physmem_get_used(void);
u64 physmem_get_total(void);

#endif

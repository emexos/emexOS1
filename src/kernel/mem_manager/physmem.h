#ifndef PHYSMEM_H
#define PHYSMEM_H

#include "../../../shared/types.h"

#define PAGE_SIZE 4096

// Physical memory management
void physmem_init(u64 mem_size);
u64 physmem_alloc_page(void);
void physmem_free_page(u64 phys_addr);
u64 physmem_get_free_pages(void);

#endif

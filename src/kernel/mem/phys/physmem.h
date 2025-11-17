#ifndef PHYSMEM_H
#define PHYSMEM_H

#include <types.h>

#define PAGE_SIZE 4096

typedef struct physmem_pageframe {
    u32 rc;
    u32 flags;
} physmem_pageframe_t;

typedef struct limine_memmap_response limine_memmap_response_t;
typedef struct limine_hhdm_response limine_hhdm_response_t;


#define FRAME_FREE       0x00    // Frame is available
#define FRAME_USED       0x01    // Frame is allocated
#define FRAME_KERNEL     0x02    // Frame contains kernel data/code
#define FRAME_USER       0x04    // Frame belongs to user process  
#define FRAME_DMA        0x08    // Frame used for DMA operations
#define FRAME_SHARED     0x10    // Frame shared between processes
#define FRAME_COW        0x20    // Copy-on-write frame
#define FRAME_CACHE      0x40    // Frame used for disk cache
#define FRAME_GUARD      0x80    // Guard page (for stack overflow)

u64 physmem_free_get(void);
u64 physmem_alloc_to(u64 count);
void physmem_free_to(u64 physmem_addr, u64 count);
void physmem_init(limine_memmap_response_t *mpr, limine_hhdm_response_t *hpr);

#endif

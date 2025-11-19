#ifndef SLAB_H
#define SLAB_H

#include <types.h>

#define PAGE_SIZE 4096
#define SLAB_MIN_SIZE 16
#define SLAB_MAX_SIZE 1024
#define SLAB_CACHE_COUNT 7  // 16, 32, 64, 128, 256, 512, 1024 bytes

typedef struct slab_allocator {
    u64 sizes[SLAB_CACHE_COUNT];
} slab_allocator_t;

enum slab_result{
    InvalidPtrSlab,
    InvalidPtrSlabMeta,
    InvalidPtrSlabData,

    SlabInitSuccess,
};

void slab_init(slab_allocator_t *ptr_slab, u64 *ptr_slab_meta, u64 *ptr_slab_data); 

#endif

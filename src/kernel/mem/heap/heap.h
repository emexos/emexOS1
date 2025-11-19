#ifndef HEAP_H
#define HEAP_H

#include <types.h>

typedef struct heap_block {
    u32 magic;
    u64 size;
    struct heap_block* next;
    struct heap_block* prev;
    u8 used;
} heap_block_t;

u64 *malloc(heap_block_t *heap, u64 size);
int free(u64 *ptr);

#define BLOCK_MAGIC 0xDEADBEEF

#endif

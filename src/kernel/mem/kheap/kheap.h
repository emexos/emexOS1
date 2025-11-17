#ifndef HEAP_H
#define HEAP_H

#include <types.h>

u64 *kmalloc(u64 size);
u64 *kcalloc(u64 num, u64 size);
u64 *krealloc(u64 *ptr, u64 size);
void kfree(u64 *ptr);

void kheap_init(void);

u64  kheap_get_total_size(void);
u64  kheap_get_used_size(void); 
u64  kheap_get_free_size(void);
u64  kheap_get_block_count(void);
u64  kheap_get_used_block_count(void);
// void kheap_print_stats(void);

#define HEAP_START 0xFFFF800000000000  // Start of kernel heap
#define HEAP_SIZE  (2 * 4096 * PAGE_SIZE) // 16MB initial heap
#define BLOCK_MAGIC 0xDEADBEEF

#endif

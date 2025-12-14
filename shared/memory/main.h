#ifndef MAIN_H
#define MAIN_H

#include <types.h>

// BASICS
void memset(void *ptr, u8 val, size_t n);
void memcpy(void *dst, const void *src, size_t n);
void memmove(void *dst, const void *src, size_t n);
int memcmp(const void *a, const void *b, size_t n);

// Memory info is used in kernel but soon in console programms
u64 mem_get_free(void);
u64 mem_get_used(void);
u64 mem_get_total(void);

// DMA memory allocation (for disk I/O) (fat32)
void* kmalloc_dma(size_t size, size_t alignment);
void kfree_dma(void *ptr);

//page aligned allocation
void* kmalloc_aligned(size_t size, size_t alignment);

#endif

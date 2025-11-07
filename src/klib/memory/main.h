#ifndef MAIN_H
#define MAIN_H

#include <types.h>

// BASICS
void memset(void *ptr, u8 val, size_t n);
void memcpy(void *dst, const void *src, size_t n);
void memmove(void *dst, const void *src, size_t n);
int memcmp(const void *a, const void *b, size_t n);

void mem_init(void *heap_start, size_t heap_sz);
void *kalloc(size_t sz);
void kfree(void *ptr);

// Memory info is used in kernel but soon in console programms
size_t mem_get_free(void);
size_t mem_get_used(void);

#endif

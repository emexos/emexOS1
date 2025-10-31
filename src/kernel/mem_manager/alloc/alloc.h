#ifndef ALLOC_H
#define ALLOC_H

#include "../../../../shared/types.h"

#define ALIGN_SIZE 16
#define MIN_BLOCK_SIZE 32

void alloc_init(void *base, size_t size);
void *kalloc(size_t sz);
void kfree(void *ptr);

size_t alloc_get_free(void);
size_t alloc_get_used(void);

#endif

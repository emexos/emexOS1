#include "main.h"

//todo: split in files
#define BLOCK_SIZE 32

typedef struct block {
    size_t size;
    int used;
    struct block *next;
} block_t;

static u8 *heap_start = NULL;
static size_t heap_size = 0;
static block_t *first_block = NULL;

static size_t free_mem = 0;
static size_t used_mem = 0;

// BASIC
void memset(void *ptr, u8 val, size_t n)
{
    u8 *p = (u8 *)ptr;
    for (size_t i = 0; i < n; i++) {
        p[i] = val;
    }
}

void memcpy(void *dst, const void *src, size_t n)
{
    u8 *d = (u8 *)dst;
    const u8 *s = (const u8 *)src;
    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
}

void memmove(void *dst, const void *src, size_t n)
{
    u8 *d = (u8 *)dst;
    const u8 *s = (const u8 *)src;

    if (d < s) {
        for (size_t i = 0; i < n; i++) {
            d[i] = s[i];
        }
    } else {
        for (size_t i = n; i > 0; i--) {
            d[i - 1] = s[i - 1];
        }
    }
}

int memcmp(const void *a, const void *b, size_t n)
{
    const u8 *pa = (const u8 *)a;
    const u8 *pb = (const u8 *)b;

    for (size_t i = 0; i < n; i++) {
        if (pa[i] != pb[i]) {
            return pa[i] - pb[i];
        }
    }
    return 0;
}


// starts a heap (like in kernel.c)
void mem_init(void *heap_addr, size_t heap_sz)
{
    heap_start = (u8 *)heap_addr;
    heap_size = heap_sz;

    first_block = (block_t *)heap_start;
    first_block->size = heap_sz - sizeof(block_t);
    first_block->used = 0;
    first_block->next = NULL;

    free_mem = first_block->size;
    used_mem = 0;
}


static block_t *find_free(size_t size)
{
    block_t *current = first_block;

    while (current != NULL) {
        if (!current->used && current->size >= size) {
            return current;
        }
        current = current->next;
    }

    return NULL;
}

static void split_block(block_t *block, size_t size)
{
    if (block->size < size + sizeof(block_t) + BLOCK_SIZE) {
        return;
    }

    block_t *new_block = (block_t *)((u8 *)block + sizeof(block_t) + size);
    new_block->size = block->size - size - sizeof(block_t);
    new_block->used = 0;
    new_block->next = block->next;

    block->size = size;
    block->next = new_block;
}

// Kernel Allocate
void *kalloc(size_t sz)
{
    if (sz == 0) {
        return NULL;
    }

    //16 bytes
    //
    sz = (sz + 15) & ~15;

    block_t *block = find_free(sz);
    if (block == NULL) {
        return NULL;
    }

    split_block(block, sz);
    block->used = 1;

    free_mem -= block->size;
    used_mem += block->size;

    void *ptr = (void *)((u8 *)block + sizeof(block_t));
    memset(ptr, 0, sz);

    return ptr;
}

// kernel Free
void kfree(void *ptr)
{
    if (ptr == NULL) {
        return;
    }

    block_t *block = (block_t *)((u8 *)ptr - sizeof(block_t));

    if (!block->used) {
        return; // Double free protection
    }

    block->used = 0;

    free_mem += block->size;
    used_mem -= block->size;

    // merge with nect block
    if (block->next != NULL && !block->next->used)
    {
        block->size += sizeof(block_t) + block->next->size;
        block->next = block->next->next;
    }
}


//stats maybe for the console
size_t mem_get_free(void) {
    return free_mem;
}

size_t mem_get_used(void) {
    return used_mem;
}

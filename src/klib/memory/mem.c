#include "main.h"

#include "../../kernel/mem_manager/alloc/alloc.h"

//todo: split in files
#define BLOCK_SIZE 32

//now in alloc.c
/*
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
*/

// BASIC
void memset(void *ptr, u8 val, size_t n)
{
    u8 *p = (u8 *)ptr;
    for (size_t i = 0; i < n; i++)
        p[i] = val;
}

void memcpy(void *dst, const void *src, size_t n)
{
    u8 *d = (u8 *)dst;
    const u8 *s = (const u8 *)src;
    for (size_t i = 0; i < n; i++)
        d[i] = s[i];
}

void memmove(void *dst, const void *src, size_t n)
{
    u8 *d = (u8 *)dst;
    const u8 *s = (const u8 *)src;

    if (d < s) {
        for (size_t i = 0; i < n; i++)
            d[i] = s[i];
    } else {
        for (size_t i = n; i > 0; i--)
            d[i - 1] = s[i - 1];
    }
}

int memcmp(const void *a, const void *b, size_t n)
{
    const u8 *pa = (const u8 *)a;
    const u8 *pb = (const u8 *)b;

    for (size_t i = 0; i < n; i++) {
        if (pa[i] != pb[i])
            return pa[i] - pb[i];
    }
    return 0;
}


void mem_init(void *heap_addr, size_t heap_sz) {
    alloc_init(heap_addr, heap_sz);
}

//stats maybe for the console
size_t mem_get_free(void) {
    return alloc_get_free();
}

size_t mem_get_used(void) {
    return alloc_get_used();
}


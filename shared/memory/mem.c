#include "main.h"
#include <kernel/mem/heap/heap.h>

#define BLOCK_SIZE 32

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

// u64 mem_get_free() {
//     return kheap_get_free_size();
// }

// u64 mem_get_used(void) {
//     return kheap_get_used_size();
// }

// u64 mem_get_total(void) {
//     return kheap_get_total_size();
// }


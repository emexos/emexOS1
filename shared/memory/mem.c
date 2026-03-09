#include "main.h"
#include <kernel/mem/heap/heap.h>

#define BLOCK_SIZE 32

// BASIC
void memset(void *ptr, u8 val, size_t n)
{
    u8 *p = (u8 *)ptr;

    while (n && ((size_t)p & 7)) { *p++ = val; n--; }

    // fill 8 bytes at a time
    u64 fill = (u64)val * 0x0101010101010101ULL;
    u64 *p64 = (u64 *)p;
    size_t blocks = n / 8;
    while (blocks--) *p64++ = fill;

    // remaining bytes
    p = (u8 *)p64;
    n &= 7;
    while (n--) *p++ = val;
}

void memcpy(void *dst, const void *src, size_t n)
{
    u8 *d = (u8 *)dst;
    u64 *d64 = (u64 *)d;
    const u8 *s = (const u8 *)src;
    const u64 *s64 = (const u64 *)s;

    size_t blocks = n / 8; // 8 bytes

    while (n && ((size_t)d & 7)) { *d++ = *s++; n--; }
    while (blocks--) *d64++ = *s64++;

    // remaining bytes
    d = (u8 *)d64;
    s = (const u8 *)s64;
    n &= 7;
    while (n--) *d++ = *s++;
}

void memmove(void *dst, const void *src, size_t n)
{
    u8 *d = (u8 *)dst;
    const u8 *s = (const u8 *)src;

    if (d == s || n == 0) return;

    if (d < s) {
        // forward copy: align then 8-byte chunks
        while (n && ((size_t)d & 7)) { *d++ = *s++; n--; }
        u64 *d64 = (u64 *)d;
        const u64 *s64 = (const u64 *)s;
        size_t blocks = n / 8;
        while (blocks--) *d64++ = *s64++;
        d = (u8 *)d64; s = (const u8 *)s64; n &= 7;
        while (n--) *d++ = *s++;
    } else {
        // backward copy to handle overlap
        d += n; s += n;
        while (n && ((size_t)d & 7)) { *--d = *--s; n--; }
        u64 *d64 = (u64 *)d;
        const u64 *s64 = (const u64 *)s;
        size_t blocks = n / 8;
        while (blocks--) *--d64 = *--s64;
        d = (u8 *)d64; s = (const u8 *)s64; n &= 7;
        while (n--) *--d = *--s;
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


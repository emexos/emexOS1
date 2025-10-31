#include "physmem.h"

#define MAX_PAGES 65536
#define BITMAP_SIZE (MAX_PAGES / 8)
#define KERNEL_RESERVED_PAGES 1024

static u8 bitmap[BITMAP_SIZE];
static u64 total, free, used;

static inline void mark_used(u64 idx) {
    bitmap[idx >> 3] |= (1 << (idx & 7));
}

static inline void mark_free(u64 idx) {
    bitmap[idx >> 3] &= ~(1 << (idx & 7));
}

static inline int is_free(u64 idx) {
    return !(bitmap[idx >> 3] & (1 << (idx & 7)));
}

void physmem_init(u64 mem_size) {
    total = mem_size / PAGE_SIZE;
    if (total > MAX_PAGES) total = MAX_PAGES;

    for (u64 i = 0; i < BITMAP_SIZE; i++)
        bitmap[i] = 0;

    free = total;
    used = 0;

    for (u64 i = 0; i < KERNEL_RESERVED_PAGES; i++) {
        mark_used(i);
        free--;
        used++;
    }
}

u64 physmem_alloc_page(void) {
    if (free == 0) return 0;

    for (u64 i = 0; i < total; i++) {
        if (is_free(i)) {
            mark_used(i);
            free--;
            used++;
            return i * PAGE_SIZE;
        }
    }
    return 0;
}

u64 physmem_alloc_pages(u32 n) {
    if (n == 0 || free < n) return 0;

    for (u64 i = 0; i <= total - n; i++) {
        u32 j;
        for (j = 0; j < n; j++) {
            if (!is_free(i + j)) break;
        }

        if (j == n) {
            for (u32 k = 0; k < n; k++) {
                mark_used(i + k);
                free--;
                used++;
            }
            return i * PAGE_SIZE;
        }
    }
    return 0;
}

void physmem_free_page(u64 addr) {
    u64 pg = addr / PAGE_SIZE;
    if (pg >= total || is_free(pg)) return;

    mark_free(pg);
    free++;
    used--;
}

void physmem_get_free_pages(u64 addr, u32 n) {
    for (u32 i = 0; i < n; i++)
        physmem_free_page(addr + i * PAGE_SIZE);
}

u64 physmem_get_free(void) { return free; }
u64 physmem_get_used(void) { return used; }
u64 physmem_get_total(void) { return total; }

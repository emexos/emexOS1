#include "physmem.h"

#define MAX_PAGES 65536  // 256 MB / 4096
#define BITMAP_SIZE (MAX_PAGES / 8)

static u8 bitmap[BITMAP_SIZE];
static u64 total_pages = 0;
static u64 free_pages = 0;

void physmem_init(u64 mem_size) {
    total_pages = mem_size / PAGE_SIZE;

    if (total_pages > MAX_PAGES) {
        total_pages = MAX_PAGES;
    }

    for (u64 i = 0; i < BITMAP_SIZE; i++) {
        bitmap[i] = 0;
    }

    free_pages = total_pages;


    // Reserve first 1024 pages (4 MB) for kernel
    for (u64 i = 0; i < 1024; i++) {
        u64 byte = i / 8;
        u64 bit = i % 8;
        bitmap[byte] |= (1 << bit);
    }

    free_pages -= 1024;
}

u64 physmem_alloc_page(void) {
    if (free_pages == 0) {
        return 0;
    }

    // Find free page
    for (u64 i = 0; i < total_pages; i++) {
        u64 byte = i / 8;
        u64 bit = i % 8;

        if (!(bitmap[byte] & (1 << bit))) {
            bitmap[byte] |= (1 << bit);
            free_pages--;
            return i * PAGE_SIZE;
        }
    }
    return 0;
}

void physmem_free_page(u64 phys_addr) {
    u64 page = phys_addr / PAGE_SIZE;

    if (page >= total_pages) {
        return;
    }

    u64 byte = page / 8;
    u64 bit = page % 8;

    if (!(bitmap[byte] & (1 << bit))) {
        return; // Already free
    }

    bitmap[byte] &= ~(1 << bit);
    free_pages++;
}

u64 physmem_get_free_pages(void) {
    return free_pages;
}

#include "kheap.h"
#include "../phys/physmem.h"
#include <limine/limine.h>
#include <kernel/exceptions/panic.h>
#include <klib/memory/main.h>
#include <kernel/mem/paging/paging.h>

typedef struct kheap_block {
    u32 magic;
    size_t size;
    struct kheap_block* next;
    struct kheap_block* prev;
    u8 used;
} kheap_block_t;

static kheap_block_t *kheap_start = NULL;

static struct {
    u64 total_size;
    u64 used_size;
    u64 free_size;
    u64 total_blocks;
    u64 used_blocks;
} kheap_stats;

static void kheap_merge_free_blocks(kheap_block_t *block) {
    kheap_block_t *current = block;

    while (current->next && !current->next->used) {
        current->size += sizeof(kheap_block_t) + current->next->size;
        current->next = current->next->next;
        if (current->next) {
            current->next->prev = current;
        }
        kheap_stats.total_blocks--;
        kheap_stats.free_size += sizeof(kheap_block_t);
    }

    current = block;
    while (current->prev && !current->prev->used) {
        kheap_block_t *prev = current->prev;
        prev->size += sizeof(kheap_block_t) + current->size;
        prev->next = current->next;
        if (current->next) {
            current->next->prev = prev;
        }
        kheap_stats.total_blocks--;
        kheap_stats.free_size += sizeof(kheap_block_t);
        current = prev;
    }
}

void kheap_init(void) {
    kheap_start = (kheap_block_t *)HEAP_START;
    kheap_start->magic = BLOCK_MAGIC;
    kheap_start->size = HEAP_SIZE - sizeof(kheap_block_t);
    kheap_start->used = 0;
    kheap_start->next = NULL;
    kheap_start->prev = NULL;

    kheap_stats.total_size = HEAP_SIZE;
    kheap_stats.used_size = 0;
    kheap_stats.free_size = kheap_start->size;
    kheap_stats.total_blocks = 1;
    kheap_stats.used_blocks = 0;
}

u64 *kmalloc(u64 size) {
    if (size == 0) return NULL;

    size = (size + 7) & ~7;

    kheap_block_t *current = kheap_start;
    kheap_block_t *best_fit = NULL;

    while (current != NULL) {
        if (!current->used && current->size >= size) {
            best_fit = current;
            break;
        }
        current = current->next;
    }

    if (best_fit == NULL) return NULL;

    if (best_fit->size >= size + sizeof(kheap_block_t) + 16) {
        kheap_block_t *new_block = (kheap_block_t *)((u8 *)best_fit + sizeof(kheap_block_t) + size);

        new_block->magic = BLOCK_MAGIC;
        new_block->size = best_fit->size - size - sizeof(kheap_block_t);
        new_block->used = 0;
        new_block->next = best_fit->next;
        new_block->prev = best_fit;

        if (best_fit->next) {
            best_fit->next->prev = new_block;
        }

        best_fit->size = size;
        best_fit->next = new_block;

        kheap_stats.total_blocks++;
        kheap_stats.free_size -= sizeof(kheap_block_t);
    }

    best_fit->used = 1;

    kheap_stats.used_size += best_fit->size;
    kheap_stats.free_size -= best_fit->size;
    kheap_stats.used_blocks++;

    return (u64 *)((u8 *)best_fit + sizeof(kheap_block_t));
}

void kfree(u64 *ptr) {
  if (ptr == NULL) return;

  kheap_block_t *blk = (kheap_block_t *) ((u8 *)ptr - sizeof(kheap_block_t));

  if (blk->magic != BLOCK_MAGIC) {
      //printf("\n\nERROR: kernel invalid blk! Doube free?\n");
      return;
  }

  if (!blk->used) {
      //printf("\n WARNING: Block already freed\n");
      return;
  }

  blk->used = 0;
  kheap_stats.used_size -= blk->size;
  kheap_stats.free_size += blk->size;
  kheap_stats.used_blocks--;

  kheap_merge_free_blocks(blk);
}


u64 kheap_get_total_size(void) {
    return kheap_stats.total_size;
}

u64 kheap_get_used_size(void) {
    return kheap_stats.used_size;
}

u64 kheap_get_free_size(void) {
    return kheap_stats.free_size;
}

u64 kheap_get_block_count(void) {
    return kheap_stats.total_blocks;
}

u64 kheap_get_used_block_count(void) {
    return kheap_stats.used_blocks;
}

u64 *kcalloc(u64 count, u64 size) {
    u64 total = count * size;
    u64 *ptr = kmalloc(total);
    if (ptr) memset(ptr, 0, total);
    return ptr;
}

u64 *krealloc(u64 *ptr, u64 size) {
    if (!ptr) return kmalloc(size);
    if (size == 0) {
        kfree(ptr);
        return NULL;
    }

    kheap_block_t *blk = (kheap_block_t *)((u8 *)ptr - sizeof(kheap_block_t));
    if (blk->magic != BLOCK_MAGIC) return NULL;
    if (size <= blk->size) return ptr;

    u64 *ptr_new = kmalloc(size);
    if (ptr_new) {
        memcpy(ptr_new, ptr, blk->size);
        kfree(ptr);
    }
    return ptr_new;
}

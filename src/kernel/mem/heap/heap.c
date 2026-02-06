#include "heap.h"
#include "../phys/physmem.h"
#include <limine/limine.h>
#include <kernel/arch/x64/exceptions/panic.h>
#include <memory/main.h>
#include <kernel/mem/paging/paging.h>
#include <kernel/communication/serial.h>
#include <kernel/graph/theme.h>
#include <theme/doccr.h>

static int heap_merge_free_blocks(heap_block_t *block) {
    if (block->magic != BLOCK_MAGIC) return 0;
    int ret = 0;

    heap_block_t *current = block;

    while (current->next && !current->next->used && current->next->magic == BLOCK_MAGIC) {

        heap_block_t *next = current->next;

        current->size += sizeof(heap_block_t) + next->size;
        current->next = next->next;

        if (current->next) {
            if (current->next->magic == BLOCK_MAGIC) {
                current->next->prev = current;
            }
        }

        ret++;
   }

    current = block;
    while (current->prev && !current->prev->used && current->prev->magic == BLOCK_MAGIC) {

        heap_block_t *prev = current->prev;

        prev->size += sizeof(heap_block_t) + current->size;
        prev->next = current->next;

        if (prev->next) {
            if (prev->next->magic == BLOCK_MAGIC) {
                prev->next->prev = prev;
            }
        }

        ret++;

        current = prev;
    }

    return ret;
}

u64 *malloc(heap_block_t *heap, u64 size) {
    if (!heap || size == 0) return NULL;

    size = (size + 15) & ~15;

    heap_block_t *current = heap;
    heap_block_t *best_fit = NULL;
    size_t best_size = SIZE_MAX;

    while (current != NULL) {
        if (!current->used && current->size >= size && current->size < best_size) {
            best_fit = current;
            best_size = current->size;

            if (current->size == size) break;
        }
        current = current->next;
    }

    if (best_fit == NULL) return NULL;

    if (best_fit->size >= size + sizeof(heap_block_t) + 16) {
        heap_block_t *new_block = (heap_block_t *)((u8 *)best_fit + sizeof(heap_block_t) + size);

        new_block->magic = BLOCK_MAGIC;
        new_block->size = best_fit->size - size - sizeof(heap_block_t);
        new_block->used = 0;
        new_block->next = best_fit->next;
        new_block->prev = best_fit;

        if (best_fit->next) {
            best_fit->next->prev = new_block;
        }

        best_fit->size = size;
        best_fit->next = new_block;
    }

    best_fit->used = 1;

    return (u64 *)((u8 *)best_fit + sizeof(heap_block_t));
}

int free(u64 *ptr) {
  if (!ptr) return 0;

  heap_block_t *blk = (heap_block_t *) ((u8 *)ptr - sizeof(heap_block_t));

  if (blk->magic != BLOCK_MAGIC) {
      BOOTUP_PRINTF("\n\nERROR: kernel invalid blk! Doube free?\n");
      return 0;
  }

  if (!blk->used) {
      BOOTUP_PRINTF("\n WARNING: Block already freed\n");
      return 0;
  }

  if (blk->size > (1ULL << 40)) {
      BOOTUP_PRINTF("\n ERROR: implausible block size\n");
      return 0;
  }

  blk->used = 0;

  return heap_merge_free_blocks(blk);
}

#include "physmem.h"
#include <limine.h>
#include <kernel/exceptions/panic.h>
#include <klib/memory/main.h>

static struct physmem_pageframe *physmem_pageframes = NULL;
static u64 physmem_total = 0;
static u64 physmem_free  = 0;
static u64 physmem_used  = 0;
static u8 *bitmap        = NULL;
static u64 bitmap_size   = 0;

static u64 physmem_addr_highest = 0;

static inline int bitmap_test(u64 idx) {
    return (bitmap[idx >> 3] >> (idx & 7)) & 1;
}

static inline void bitmap_set(u64 idx) {
    bitmap[idx >> 3] |= (1 << (idx & 7));
}

static inline void bitmap_clear(u64 idx) {
    bitmap[idx >> 3] &= ~(1 << (idx & 7));
}

/// Summary
/// 2025/11/17 tsaraki
/// marking all tre memory that hardware is providing
/// and limine mapped
static void physmem_addr_mark(limine_memmap_response_t *mpr) {

    if (!physmem_pageframes) {
        panic("PHYSMEM ADDR MARK PHYSMEM PAGEFRAMES NULL");
        return;
    }

    for (u64 i = 0; i < mpr->entry_count; i++) {
        struct limine_memmap_entry *entry = mpr->entries[i];

        u64 frame_start = entry->base / PAGE_SIZE;
        if (frame_start >= physmem_total) continue;

        u64 frame_end = (entry->base + entry->length) / PAGE_SIZE;

        if (frame_end > physmem_total) frame_end = physmem_total;
        
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            for (u64 frame = frame_start; frame < frame_end; frame++) {
                bitmap_clear(frame);
                physmem_pageframes[frame].rc = 0;
                physmem_pageframes[frame].flags = FRAME_FREE;
            }
        } else {
            for (u64 frame = frame_start; frame < frame_end; frame++) {
                bitmap_set(frame);
                physmem_pageframes[frame].rc = 1;
                physmem_pageframes[frame].flags = FRAME_USED | FRAME_KERNEL;
            }
        }
    }
}

void physmem_addr_mark_used(u64 physmem_addr, u64 count) {

    if (!physmem_pageframes) {
        panic("PHYSMEM ADDR MARK USED PHYSMEM PAGEFRAMES NULL");
        return;
    }

    u64 frame_start = physmem_addr / PAGE_SIZE;
    u64 frame_end = frame_start + count;

    if (frame_end > physmem_total) frame_end = physmem_total;
    
    for (u64 frame = frame_start; frame < frame_end; frame++) {
        bitmap_set(frame);

        physmem_pageframes[frame].rc = 1;
        physmem_pageframes[frame].flags = FRAME_USED;
    }
}

void physmem_addr_mark_free(u64 physmem_addr, u64 count) {

    if (!physmem_pageframes) {
        panic("PHYSMEM ADDR MARK FREE PHYSMEM PAGEFRAMES NULL");
        return;
    }

    u64 frame_start = physmem_addr / PAGE_SIZE;
    u64 frame_end = frame_start + count;

    if (frame_end > physmem_total) frame_end = physmem_total;
    
    for (u64 frame = frame_start; frame < frame_end; frame++) {
        bitmap_clear(frame);

        physmem_pageframes[frame].rc = 0;
        physmem_pageframes[frame].flags = FRAME_FREE;
    }
}


/// Summary
/// 2025/11/17 tsaraki
/// this is the first addres that the kernel is getting
/// should be used once in physmem_init
void *physmem_addr_get_tracking(
    limine_memmap_response_t *mpr,
    limine_hhdm_response_t *hpr,
    u64 size
) {
    if (!mpr) panic("PHYSMEM ADDR GET LIMINE MEMMAP EQ NULL\n");
    if (!hpr) panic("PHYSMEM ADDR GET LIMINE HHDM EQ NULL\n");

    for (u64 i = 0; i < mpr->entry_count; i++) {
        struct limine_memmap_entry *entry = mpr->entries[i];

        if (entry->type == LIMINE_MEMMAP_USABLE && 
            entry->length >= size &&
            entry->base >= 0x1000000) {

            return (void *)entry->base;
        }
    }

    for (u64 i = 0; i < mpr->entry_count; i++) {
        struct limine_memmap_entry *entry = mpr->entries[i];

        if (entry->type == LIMINE_MEMMAP_USABLE && 
            entry->length >= size &&
            entry->base >= 0x100000) {
            return (void *)entry->base;
        }
    }

    for (u64 i = 0; i < mpr->entry_count; i++) {
        struct limine_memmap_entry *entry = mpr->entries[i];

        if (entry->type == LIMINE_MEMMAP_USABLE && 
            entry->length >= size) {
            return (void *)entry->base;
        }
    }

    /// 2025/11/17 tsaraki
    // panic in physmem_init
    return NULL;
}

/// Summary
/// 2025/11/17/// 2025/11/17 tsaraki
/// helper, used once in physmem_init
u64 used_bytes_to_frame_count(u64 size)  {
    return (size + PAGE_SIZE - 1) / PAGE_SIZE;
}

/// Summary
/// 2025/11/17 tsaraki
/// function that setup a physical memory
/// have to be called once in _start() kernel.c
void physmem_init(limine_memmap_response_t *mpr, limine_hhdm_response_t *hpr) {
    physmem_addr_highest = 0;
    
    for (u64 i = 0; i < mpr->entry_count; i++) {
        struct limine_memmap_entry *entry = mpr->entries[i];

        if (entry->type == LIMINE_MEMMAP_USABLE ||
            entry->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE) {

            u64 end_addr = entry->base + entry->length;
        
            if (end_addr > physmem_addr_highest) {
                physmem_addr_highest = end_addr;
            }
        }
    }
    
    physmem_total = physmem_addr_highest / PAGE_SIZE;

    u64 physmem_size = physmem_total * sizeof(physmem_pageframe_t);
    bitmap_size = (physmem_total + 7) / 8;
    
    void *physmem_pageframe_addr = physmem_addr_get_tracking(mpr, hpr, physmem_size + bitmap_size);
    if (!physmem_pageframe_addr) panic("No usable memory found for physmem_pageframes");

    void *physmem_pageframes_vaddr = (void *)((u64)physmem_pageframe_addr + hpr->offset);
    physmem_pageframes = (physmem_pageframe_t *)physmem_pageframes_vaddr;

    void *bitmap_addr = (void *)((u64)physmem_pageframe_addr + physmem_size);
    bitmap = (u8 *)((u64)bitmap_addr + hpr->offset);
      
    memset(bitmap, 0xFF, bitmap_size);
    
    physmem_addr_mark(mpr);

    u64 to_used = used_bytes_to_frame_count(physmem_size + bitmap_size);
    physmem_addr_mark_used((u64)physmem_pageframe_addr, to_used);
}

/// Summary
/// 2025/11/17 tsaraki
/// @Note @Important @Behavior
/// if it necessary to allocate physical memory in a large amount
/// DO NOT DO IT BY 1 FN CALL IN A LOOP
/// do it like physmem_alloc_to(4096)
/// and then page map it in a loop iterating over i of range 0..<4096
u64 physmem_alloc_to(u64 count) { //count is len of frames of size 4096
    if (count == 0) return 0;
    
    u64 consecutive = 0;
    u64 start_frame = 0;
    u8  all_free = 0;

    for (u64 frame = 0; frame < physmem_total; frame++) {
        if (!bitmap_test(frame)) {
            if (consecutive == 0) {
                start_frame = frame;
            }
            consecutive++;

            if (consecutive >= count) {
                all_free = 1;
                break;
            }
        } else {
            consecutive = 0;
        }
    }

    if (all_free) {
        u64 physmem_addr = start_frame * PAGE_SIZE;
        physmem_addr_mark_used(physmem_addr, count);
        
        return physmem_addr;
    }

    return 0;
}

/// Summary
/// 2025/11/17 tsaraki
/// @Note @Important @Behavior
/// if it necessary to free physical memory in a large amount
/// DO NOT DO IT BY 1 FN CALL IN A LOOP
/// do it like physmem_free_to(4096)
void physmem_free_to(u64 physmem_addr, u64 count) { //count is len of frames of size 4096
    if (count == 0) return;

    u64 frame_start = physmem_addr / PAGE_SIZE;
    physmem_addr_mark_free(physmem_addr, count );
}

/// Summary
/// 2025/11/17 tsaraki
/// helper stat
u64 physmem_free_get(void) {
    u64 count = 0;
    for (u64 frame = 0; frame < physmem_total; frame++) {
        if (!bitmap_test(frame)) {
            count++;
        }
    }
    return count;
}

#ifndef MEM_H
#define MEM_H

#include <types.h>
#include <limine/limine.h>

typedef struct limine_memmap_response limine_memmap_response_t;
typedef struct limine_hhdm_response limine_hhdm_response_t;

typedef struct limine_framebuffer_response limine_framebuffer_response_t;
typedef struct limine_framebuffer limine_framebuffer_t;

#define HEAP_START 0xFFFF800000000000 // Start of kernel heap
#define HEAP_SIZE 1024 * 1024 * 32

#define GRAPHICS_START (HEAP_START + HEAP_SIZE) // Start of graphics memory
#define GRAPHICS_SIZE 1024 * 1024 * 32

#define ULIME_START (GRAPHICS_START + GRAPHICS_SIZE)
#define ULIME_META_SIZE (1024 * 1024 * 2) //2mb

#define KLIME_SIZE_SLAB   (1024 * 1024 * 8)
#define KLIME_SIZE_IO     (1024 * 1024 * 4)
#define KLIME_SIZE_DMA    (1024 * 1024 * 2)
#define KLIME_SIZE_NOTHEAP (PAGE_SIZE + PAGE_SIZE + KLIME_SIZE_SLAB + KLIME_SIZE_IO + KLIME_SIZE_DMA)
#define KLIME_SIZE_HEAP HEAP_SIZE - KLIME_SIZE_NOTHEAP

#define KLIME_OFFSET_SLAB_META PAGE_SIZE
#define KLIME_OFFSET_SLAB_DATA KLIME_OFFSET_SLAB_META + PAGE_SIZE
#define KLIME_OFFSET_IO        KLIME_OFFSET_SLAB_DATA + KLIME_SIZE_SLAB
#define KLIME_OFFSET_DMA       KLIME_OFFSET_IO        + KLIME_SIZE_IO
#define KLIME_OFFSET_HEAP      KLIME_OFFSET_DMA       + KLIME_SIZE_DMA

#define GLIME_SIZE_META (PAGE_SIZE * 256) //2mb
#define GLIME_HEAP_SIZE (GRAPHICS_SIZE - GLIME_SIZE_META) 

#define PAGE_SIZE 4096
#define FRAME_FREE       0x00    // Frame is available
#define FRAME_USED       0x01    // Frame is allocated
#define FRAME_KERNEL     0x02    // Frame contains kernel data/code
#define FRAME_USER       0x04    // Frame belongs to user process
#define FRAME_DMA        0x08    // Frame used for DMA operations
#define FRAME_SHARED     0x10    // Frame shared between processes
#define FRAME_COW        0x20    // Copy-on-write frame
#define FRAME_CACHE      0x40    // Frame used for disk cache
#define FRAME_GUARD      0x80    // Guard page (for stack overflow)

typedef struct glime_request {
    limine_framebuffer_response_t *fbr;
    u64 virt;
    u64 size;
} glime_request_t;

typedef struct glime_response {
    u64 *start_framebuffer;
    u64 width;
    u64 height;
    u64 pitch;
    u16 bpp;
    u16 memory_model;
    u8 red_mask_size;
    u8 red_mask_shift;
    u8 green_mask_size;
    u8 green_mask_shift;
    u8 blue_mask_size;
    u8 blue_mask_shift;
} glime_response_t;

typedef struct klime_request {
    limine_hhdm_response_t *hpr;
    u64 phys;
    u64 virt;
    u64 size;
} klime_request_t;

typedef struct klime_response {
    u64 *ptr;
    u64 size;
} klime_response_t;

#endif

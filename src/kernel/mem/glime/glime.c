#include "glime.h"
#include <kernel/exceptions/panic.h>
#include "../heap/heap.h"
#include <klib/string/string.h>
#include <klib/memory/main.h>
#include <klib/debug/serial.h>
#include <klib/graphics/theme.h>
#include <theme/doccr.h>

glime_t *glime_init(glime_response_t *gr, u64 *ptr, u64 size) {
    if (!gr) {
        BOOTUP_PRINTF("ERROR: Invlalid glime init glime response ptr is null");
        panic( "ERROR: Invlalid glime init glime response ptr is null");
    }

    if (!ptr) {
        BOOTUP_PRINTF("ERROR: Invlalid glime init ptr to memory is null");
        panic( "ERROR: Invlalid glime init ptr to memory is null");
    }

    u64 framebuffer_len = sizeof(u32) * gr->width * gr->height;
    if (size < sizeof(heap_block_t) + GLIME_SIZE_META + framebuffer_len) {
        BOOTUP_PRINTF("ERROR: Invlalid glime init size lt needed");
        panic( "ERROR: Invlalid glime init size lt needed");
    }

    glime_t *glime = (glime_t *)ptr;

    glime->glres.start_framebuffer = gr->start_framebuffer;
    glime->glres.width = gr->width;
    glime->glres.height = gr->height;
    glime->glres.pitch = gr->pitch;


    glime->total_heap = GLIME_HEAP_SIZE - sizeof(heap_block_t) - framebuffer_len;
    glime->used_heap = 0;

    glime->start_heap = (heap_block_t *)((u64)glime + GLIME_SIZE_META + framebuffer_len);
    glime->start_heap->magic = BLOCK_MAGIC;
    glime->start_heap->size = glime->total_heap;
    glime->start_heap->next = NULL;
    glime->start_heap->prev = NULL;
    glime->start_heap->used = 0;

    glime->framebuffer_len = framebuffer_len;
    glime->framebuffer = (u32 *)glime_create(glime, framebuffer_len);
    if (!glime->framebuffer) {
        BOOTUP_PRINTF("ERROR: Invlalid glime init framebuffer is not initialized");
        panic( "ERROR: Invlalid glime init framebuffer is not initialized");
    }

    //@Temp
    u64 workspaces_total = 1;
    glime->workspaces = (gworkspace_t **) glime_alloc(glime, sizeof(gworkspace_t), workspaces_total);
    if (!glime->workspaces) {
        BOOTUP_PRINTF("ERROR: Invlalid glime init workspaces is not initialized");
        panic( "ERROR: Invlalid glime init workspaces is not initialized");
    }

    glime->workspaces_len = 0;
    glime->workspaces_total = workspaces_total ;

    return glime;
}

// int glime_session_set_active_pos(glime_t *glime, u64 pos) {
//     if (pos > GSESSION_LEN_MAX) return 1;
//     glime->active = pos;
//     return 0;
// }

// int glime_session_set_active_name(glime_t *glime, u8 *name) {
//     if (!glime || !name) return 1;
//     for (int i = 0; i < glime->sessions_len; i++) {
//         int seek = 0;
//         int eq = 1;
//         u8 *session_name = glime->sessions[i].name;
//         while (session_name[seek] != '\0') {
//             if (name[seek] == '\0') {
//                 eq = 0;
//                 break;
//             }
//             if (name[seek] != session_name[seek]) {
//                 eq = 0;
//                 break;
//             }
//         }
//         if (eq) {
//             glime->active = i;
//             return 0;
//         }
//     }
//     return 1;
// }

void glime_commit(glime_t *glime) {
    u32 *fb = (u32 *)glime->glres.start_framebuffer;

    //@TODO: dirty flags
    memcpy(fb, glime->framebuffer, glime->framebuffer_len);
}

u64 *glime_create(glime_t *glime, u64 size) {
    if (!glime || size == 0) return NULL;

    u64 *ptr = malloc(glime->start_heap, size);
    if (!ptr) return NULL;

    heap_block_t *block = (heap_block_t*)((u8*)ptr - sizeof(heap_block_t));
    glime->used_heap += block->size + sizeof(heap_block_t);
    return ptr;
}

u64 *glime_alloc(glime_t *glime, u64 size, u64 count) {
    if (!glime || size == 0) return NULL;

    u64 total = count * size;
    u64 *ptr = malloc(glime->start_heap, total);
    if (!ptr) return NULL;
    memset(ptr, 0, total);

    heap_block_t *block = (heap_block_t*)((u8*)ptr - sizeof(heap_block_t));
    glime->used_heap += block->size + sizeof(heap_block_t);
    return ptr;
}

void glime_free(glime_t *glime, u64 *ptr) {
    if (!glime || !ptr) return;

    heap_block_t *block = (heap_block_t*)((u8*)ptr - sizeof(heap_block_t));
    glime->used_heap -= block->size;
    int merged = free(ptr);
    glime->used_heap -= sizeof(heap_block_t) * merged;
}

u64 glime_get_total_size(glime_t *glime) {
    return glime->total_heap;
}

u64 glime_get_used_size(glime_t *glime) {
    return glime->used_heap;
}

u64 glime_get_free_size(glime_t *glime) {
    return glime->total_heap - glime->used_heap;
}

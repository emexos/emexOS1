#include "glime.h"
#include <kernel/arch/x86_64/exceptions/panic.h>
#include "../heap/heap.h"
#include <string/string.h>
#include <memory/main.h>
#include <kernel/communication/serial.h>
#include <kernel/graph/theme.h>
#include <theme/doccr.h>
#include <kernel/include/ports.h>

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

    //@Temp
    glime->dirty_boxes = (gbox_t *) glime_alloc(glime, sizeof(gbox_t), 256);
    glime->dirty_boxes_len = 0;

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

void glime_dirtybox_append(glime_t *glime, u64 x, u64 y, u64 width, u64 height) {
    gbox_t db = {
        .x = x,
        .y = y,
        .width = width,
        .height = height,
    };

    glime->dirty_boxes[glime->dirty_boxes_len] = db;
    glime->dirty_boxes_len++;
}

void glime_commit(glime_t *glime) {
    u32 *fb = (u32 *)glime->glres.start_framebuffer;

    u64 pitch_pixels = glime->glres.pitch / sizeof(u32);

    //clear all

    for (u64 i = 0; i < glime->dirty_boxes_len; i++) {
        gbox_t db = glime->dirty_boxes[i];
        for (u64 y = 0; y < db.height; y++) {
            u32 src_offset = (db.y + y) * glime->glres.width + db.x;
            u32 dst_offset = (db.y + y) * pitch_pixels + db.x;
            memset(&fb[dst_offset], 0, db.width * sizeof(u32));
            memcpy(&fb[dst_offset], &glime->framebuffer[src_offset], db.width * sizeof(u32));
        }
    }

    glime->dirty_boxes_len = 0;
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

// @TODO: should be different loop or fn or else
void glime_keyboard_handler(glime_t *glime) {
    u8 last = 0;
    while (1) {
        if ((inb(0x64) & 1) == 0) continue;
        u8 scancode = inb(0x60);
        if (scancode == last) continue;
        last = scancode;

        glime_key_event_t event = {
            .scancode = scancode & 0x7F,
            .pressed = !(scancode & 0x80),
            .modifiers = 0,
        };

        gworkspace_t *ws = glime->workspaces[glime->workspace_active];
        gsession_t *s = ws->sessions[ws->session_active];
        keyboard_put(s->kbrb, event);
        u8 c = keyboard_event_to_char(event);
        // printf("get char %i\n", c);

        //@TODO: something else
        gsession_clear(s, 0x00000000);
        glime_dirtybox_append(glime, s->box.x, s->box.y, 8, 8);
        gsession_put_at_char_dummy(s, c, 0, 0, 0x00FFFFFF);

        glime_commit(glime);
    }
}
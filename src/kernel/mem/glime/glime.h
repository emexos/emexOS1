#ifndef GLIME_H
#define GLIME_H

#include <types.h>
#include <../mem.h>
#include <../heap/heap.h>

typedef struct gbox {
    u64 x;
    u64 y;
    u64 width;
    u64 height;
} gbox_t;

typedef struct gsession {
    struct gworkspace *gworkspace;
    u8 name[64];
    gbox_t box;
} gsession_t;

#define GSESSION_LEN_MAX 16
#define SESSION_NAME_MAX 64

#define GWORKSPACE_NAME_MAX 64
#define GWORKSPACE_LEN_MAX 64

typedef struct gworkspace {
    struct glime *glime;
    u8 name[64];
    u64 pos_x;
    u64 pos_y;
    u64 gap_x;
    gsession_t **sessions;
    u64 sessions_len;
    u64 sessions_total;

} gworkspace_t;

typedef struct glime {
    glime_response_t glres;
    u32 *framebuffer;
    u64 framebuffer_len;
    gworkspace_t **workspaces;
    u64 workspaces_len;
    u64 workspaces_total;

    //heap
    heap_block_t *start_heap;
    u64 total_heap;
    u64 used_heap;
} glime_t;


glime_t *glime_init(glime_response_t *gr, u64 *ptr, u64 size);

void glime_commit(glime_t *glime);

//glime heap
u64 *glime_create(glime_t *glime, u64 size);
u64 *glime_alloc(glime_t *glime, u64 size, u64 count);
void glime_free(glime_t *glime, u64 *ptr);

u64 glime_get_total_size(glime_t *glime);
u64 glime_get_used_size(glime_t *glime);
u64 glime_get_free_size(glime_t *glime);

//workspace
int gworkspace_init(glime_t *glime, u8 *name, u64 pos);
int gworkspace_posx_get(gworkspace_t *gworkspace, u64 *out);
gworkspace_t *gworkspace_get_name(glime_t *glime, u8 *workspace_name);
gworkspace_t *gworkspace_get_pos(glime_t *glime, u64 pos);

//session
gsession_t *gsession_init(glime_t *glime, u8 *name, u64 width);
int gsession_attach(glime_t *glimek, gsession_t *gsession, u8 *workspace_name);
int gsession_detach(gworkspace_t *gworkspace, gsession_t *gsession);
int gsession_reattach(glime_t *glimek, gsession_t *gsession, u8 *workspace_name);

void gsession_clear(gsession_t *gsession, u32 color);
void gsession_put_at_string_dummy(gsession_t *gsession, u8 *string, u32 x, u32 y, u32 color);

#endif


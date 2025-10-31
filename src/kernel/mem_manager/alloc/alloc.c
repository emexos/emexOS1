#include "alloc.h"
#include "../../../libs/memory/main.h"

typedef struct blk {
    size_t sz;
    int used;
    struct blk *next;
} blk_t;

static u8 *base;
static size_t total_sz;
static blk_t *head;
static size_t free_sz, used_sz;

void alloc_init(void *b, size_t sz) {
    base = (u8 *)b;
    total_sz = sz;

    head = (blk_t *)base;
    head->sz = sz - sizeof(blk_t);
    head->used = 0;
    head->next = NULL;

    free_sz = head->sz;
    used_sz = 0;
}

static blk_t *find_fit(size_t sz) {
    blk_t *cur = head;
    blk_t *best = NULL;
    size_t best_sz = (size_t)-1;

    while (cur) {
        if (!cur->used && cur->sz >= sz && cur->sz < best_sz) {
            best = cur;
            best_sz = cur->sz;
        }
        cur = cur->next;
    }
    return best;
}

static void split(blk_t *b, size_t sz) {
    if (b->sz < sz + sizeof(blk_t) + MIN_BLOCK_SIZE)
        return;

    blk_t *nb = (blk_t *)((u8 *)b + sizeof(blk_t) + sz);
    nb->sz = b->sz - sz - sizeof(blk_t);
    nb->used = 0;
    nb->next = b->next;

    b->sz = sz;
    b->next = nb;
}

static void merge(void) {
    blk_t *cur = head;

    while (cur && cur->next) {
        if (!cur->used && !cur->next->used) {
            cur->sz += sizeof(blk_t) + cur->next->sz;
            cur->next = cur->next->next;
        } else {
            cur = cur->next;
        }
    }
}

void *kalloc(size_t sz) {
    if (sz == 0) return NULL;

    sz = (sz + ALIGN_SIZE - 1) & ~(ALIGN_SIZE - 1);

    blk_t *b = find_fit(sz);
    if (!b) return NULL;

    split(b, sz);
    b->used = 1;

    free_sz -= b->sz;
    used_sz += b->sz;

    void *ptr = (void *)((u8 *)b + sizeof(blk_t));
    memset(ptr, 0, sz);

    return ptr;
}

void kfree(void *ptr) {
    if (!ptr) return;

    blk_t *b = (blk_t *)((u8 *)ptr - sizeof(blk_t));
    if (!b->used) return;

    b->used = 0;
    free_sz += b->sz;
    used_sz -= b->sz;

    merge();
}

size_t alloc_get_free(void) { return free_sz; }
size_t alloc_get_used(void) { return used_sz; }

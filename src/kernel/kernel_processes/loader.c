#include "loader.h"

#include <kernel/communication/serial.h>
#include <string/string.h>

kproc_t *kproc_table[KPROC_MAX];
int kproc_cnt = 0;
int kproc_init_done = 0;

const char *state_str(u8 s) {
    switch (s) {
        case KPROC_STATE_EMPTY:   return "EMPTY";
        case KPROC_STATE_READY:   return "READY";
        case KPROC_STATE_RUNNING: return "RUNNING";
        case KPROC_STATE_BLOCKED: return "BLOCKED";
        case KPROC_STATE_DONE:    return "DONE";
        case KPROC_STATE_FAILED:  return "FAILED";
        default: return "?";
    }
}


void kproc_loader_init(void) {
    for (int i = 0; i < KPROC_MAX; i++)
        kproc_table[i] = NULL;
    kproc_cnt = 0;
    kproc_init_done = 1;
}
int kproc_register(kproc_t *proc) {
    if (!proc) return -1;
    if (!kproc_init_done) kproc_loader_init();
    if (kproc_cnt >= KPROC_MAX) return -1;

    int id = kproc_cnt;
    proc->id = (u32)id;
    proc->state = KPROC_STATE_READY;

    kproc_table[id] = proc;
    kproc_cnt++;
    return id;
}
int kproc_start(u32 id) {
    if (id >= (u32)kproc_cnt) return KPROC_ERROR;
    kproc_t *p = kproc_table[id];
    if (!p) return KPROC_ERROR;
    if (p->state != KPROC_STATE_READY) return KPROC_ERROR;

    p->state = KPROC_STATE_RUNNING;

    if (p->init) {
        int r = p->init(p);
        if (r == KPROC_ERROR) {
            p->state = KPROC_STATE_FAILED;
            if (p->fini) p->fini(p);
            return KPROC_ERROR;
        }
    }
    return KPROC_EFINE;
}
int kproc_register_and_start(kproc_t *proc) {
    int id = kproc_register(proc);
    if (id < 0) return -1;
    if (kproc_start((u32)id) != KPROC_EFINE) return -1;
    return id;
}
int kproc_tick(u32 id) {
    if (id >= (u32)kproc_cnt) return KPROC_ERROR;
    kproc_t *p = kproc_table[id];
    if (!p || p->state != KPROC_STATE_RUNNING) return KPROC_ERROR;
    if (!p->tick) return KPROC_EFINE;

    int r = p->tick(p);

    if (r == KPROC_ERROR) {
        p->state = KPROC_STATE_FAILED;
        if (p->fini) p->fini(p);
    } else if (r != KPROC_EFINE && r != KPROC_YIELD) {
        if (!(p->flags & KPROC_FLAG_PERMANENT)) {
            p->state = KPROC_STATE_DONE;
            if (p->fini) p->fini(p);
        }
    }
    return r;
}
void kproc_stop(u32 id) {
    if (id >= (u32)kproc_cnt) return;
    kproc_t *p = kproc_table[id];
    if (!p || p->state == KPROC_STATE_DONE) return;
    if (p->fini) p->fini(p);
    p->state = KPROC_STATE_DONE;
}


kproc_t *kproc_get(u32 id) {
    if (id >= (u32)kproc_cnt) return NULL;
    return kproc_table[id];
}
kproc_t *kproc_find_by_name(const char *name) {
    if (!name) return NULL;
    for (int i = 0; i < kproc_cnt; i++) {
        if (kproc_table[i] && str_equals(kproc_table[i]->name, name))
            return kproc_table[i];
    }
    return NULL;
}


int count_kernelprocesses(void) {
    return kproc_cnt;
}

#ifndef KPROC_LOADER_H
#define KPROC_LOADER_H

#include <types.h>

#define KPROC_MAX       255
#define KPROC_NAME_LEN  15

// states
#define KPROC_STATE_EMPTY   0
#define KPROC_STATE_READY   1
#define KPROC_STATE_RUNNING 2
#define KPROC_STATE_BLOCKED 3
#define KPROC_STATE_DONE    4
#define KPROC_STATE_FAILED  5

// flags
#define KPROC_FLAG_NONE      0x00
#define KPROC_FLAG_EARLY     0x01
#define KPROC_FLAG_CRITICAL  0x02
#define KPROC_FLAG_PERMANENT 0x04

// return codes
#define KPROC_EFINE 0
#define KPROC_ERROR -1
#define KPROC_YIELD 1

typedef struct kproc kproc_t;

typedef int  (*kproc_init_fn)(kproc_t *self);
typedef int  (*kproc_tick_fn)(kproc_t *self);
typedef void (*kproc_fini_fn)(kproc_t *self);

struct kproc {
    char            name[KPROC_NAME_LEN];
    u8              state;
    u8              flags;
    u8              priority;
    u8              _pad;
    kproc_init_fn   init;
    kproc_tick_fn   tick;
    kproc_fini_fn   fini;
    void           *priv;
    u32             id;
};

extern kproc_t *kproc_table[KPROC_MAX];
extern int kproc_cnt;
extern int kproc_init_done;

const char *state_str(u8 s);

void kproc_loader_init(void);
int kproc_register(kproc_t *proc);
int kproc_start(u32 id);
int kproc_register_and_start(kproc_t *proc);
int kproc_tick(u32 id);
void kproc_stop(u32 id);
kproc_t *kproc_get(u32 id);
kproc_t *kproc_find_by_name(const char *name);

int count_kprocesses(void);
void dump_kprocesses(void);

#endif

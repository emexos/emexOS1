#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <types.h>
#include <kernel/user/ulime.h>

// minimal scheduler wrapper around ulime
typedef struct {
    ulime_t *ulime;
    u64 quantum;
    u64 ticks;
} scheduler_t;
#if ENABLE_ULIME
extern scheduler_t *scheduler;
#endif

// init and config
scheduler_t* scheduler_init(ulime_t *ulime, u64 quantum);
void scheduler_set_quantum(scheduler_t *s, u64 quantum);

// scheduling - uses ulime_schedule() internally
void scheduler_tick(scheduler_t *s);
void scheduler_yield(scheduler_t *s);


#endif

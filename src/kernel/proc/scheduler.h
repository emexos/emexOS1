#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <types.h>
#include "process.h"

typedef struct {
    proc_t *queue[32];
    int head, tail, cnt;
} sched_queue_t;


// Basic scheduler functions
void sched_init(void);
void sched_enable(void);
void sched_disable(void);
int sched_is_enabled(void);

void sched_add(proc_t *p);
void sched_remove(proc_t *p);
void sched_tick(void);
void sched_yield(void);

#endif

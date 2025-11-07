#ifndef PROCESS_H
#define PROCESS_H

#include <types.h>

// process states
typedef enum {
    PROC_READY,
    PROC_RUNNING,
    PROC_BLOCKED,
    PROC_DEAD
} proc_state_t;

// first structure but should work
typedef struct proc {
    u64 pid;
    char name[64];
    proc_state_t state;
    u64 stack_base;
    u64 stack_ptr;
    u64 entry;
    struct proc *next;
} proc_t;

void process_init(void);
proc_t *process_create(const char *name, u64 entry);
void process_destroy(proc_t *p);
proc_t *process_get_current(void);
void process_set_state(proc_t *p, proc_state_t state);

#endif

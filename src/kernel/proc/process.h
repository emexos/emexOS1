#ifndef PROCESS_H
#define PROCESS_H

#include "../../../shared/types.h"

// process states
typedef enum {
    PROC_READY,
    PROC_RUNNING,
    PROC_WAITING
} proc_state_t;

// first structure but should work
typedef struct process {
    u64 pid;
    char name[64];
    proc_state_t state;
    u64 stack;
    struct process *next;
} process_t;


void process_init(void);

#endif

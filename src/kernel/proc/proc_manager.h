#ifndef PROC_MANAGER_H
#define PROC_MANAGER_H

#include <types.h>
#include <kernel/user/ulime.h>

// minimal process manager wrapper around ulime functions
typedef struct {
    ulime_t *ulime;
} proc_manager_t;

proc_manager_t* proc_mng_init(ulime_t *ulime);

extern proc_manager_t *proc_mgr;

// process control
ulime_proc_t* proc_create_proc(proc_manager_t *pm, u8 *name, u64 entry, u64 priority);
int proc_kill_proc(proc_manager_t *pm, u64 pid);
int proc_load_program_from_ulime(proc_manager_t *pm, ulime_proc_t *proc, u8 *code, u64 size);
void proc_list_procs(proc_manager_t *pm);

// random
ulime_proc_t* proc_find(proc_manager_t *pm, u64 pid);
int proc_count(proc_manager_t *pm);

#endif

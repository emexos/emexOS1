#ifndef ULIME_H
#define ULIME_H

#include <types.h>
#include "../mem.h"
#include "../klime/klime.h"
#include "../glime/glime.h"
#include "../phys/physmem.h"


typedef struct ulime_proc {
    u64 pid;
    u8  name[64];
    u64 priority;
    u64 state;
    u64 *ptr_pagetable;
    u64 entry_point;
    u64 heap_base;
    u64 heap_size;
    u64 stack_base;
    u64 stack_size;

    u64 phys_heap;
    u64 phys_stack;

    struct ulime *ulime;
    struct ulime_proc *next;
    struct ulime_proc *prev;
} ulime_proc_t;

//@TODO: move someone else
#define SYS_EXIT    0
#define SYS_WRITE   1
#define SYS_READ    2
#define SYS_GETPID  3

typedef u64 (*syscall_handler_t)(ulime_proc_t *proc, u64 arg1, u64 arg2, u64 arg3);

typedef struct ulime_thread {
    u64 tid;
    ulime_proc_t *ptr_proc;
    u64 *ptr_stack;
    u64 *ptr_instruction;
    u64 state;
} ulime_thread_t;


typedef struct ulime {
    ulime_proc_t   *ptr_proc_list;
    ulime_proc_t   *ptr_proc_curr;
    ulime_thread_t *ptr_thread;
    u64 pid_next;
    u64 tid_next;

    klime_t *klime;
    glime_t *glime;
    limine_hhdm_response_t *hpr;


    syscall_handler_t syscalls[256];

    u64 internal_pool_base;
    u64 internal_pool_size;
    u64 internal_pool_used;

    // USER memory space management
    u64 user_space_base;
    u64 user_space_size;
    u64 user_space_used;

    //ipc
} ulime_t;



#define PROC_CREATED    1
#define PROC_READY      2
#define PROC_RUNNING    3
#define PROC_BLOCKED    4
#define PROC_ZOMBIE     5

ulime_t *ulime_init(limine_hhdm_response_t *hpr, klime_t *klime, glime_t *glime, u64 uphys_start);
void ulime_init_syscalls(ulime_t *ulime);

ulime_proc_t *ulime_proc_create(ulime_t *ulime, u8 *name, u64 entry_point);
int ulime_proc_kill(ulime_t *ulime, u64 pid);
int ulime_proc_mmap(ulime_t *ulime, ulime_proc_t *proc);
void ulime_proc_test_mem(ulime_proc_t *proc);

void ulime_schedule(ulime_t *ulime);
void ulime_proc_list(ulime_t *ulime);

int ulime_load_program(ulime_proc_t *proc, u8 *code, u64 code_size);

#endif

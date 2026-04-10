#pragma once

#include <types.h>
#include <kernel/user/ulime.h>
#include <kernel/proc/scheduler.h>
#include <kernel/arch/x86_64/idt/idt.h>

// offsets MUST match scheduler.asm exactly
typedef struct {
    u64 r15;    // 0
    u64 r14;    // 8
    u64 r13;    // 16
    u64 r12;    // 24
    u64 r11;    // 32
    u64 r10;    // 40
    u64 r9;     // 48
    u64 r8;     // 56
    u64 rbp;    // 64
    u64 rdi;    // 72
    u64 rsi;    // 80
    u64 rdx;    // 88
    u64 rcx;    // 96
    u64 rbx;    // 104
    u64 rax;    // 112
    u64 _pad0;  // 120
    u64 _pad1;  // 128
    u64 rip;    // 136
    u64 _pad2;  // 144
    u64 rflags; // 152
    u64 rsp;    // 160
} mt_context_t;

// saved user-mode cpu state
typedef struct {
    u64 r15, r14, r13, r12, r11, r10, r9, r8;
    u64 rbp, rdi, rsi, rdx, rcx, rbx, rax;
    u64 rip;
    u64 cs;
    u64 rflags;
    u64 rsp;
    u64 ss;
    u64 cr3;
    int saved; // 0 == first run; use entry_point
} mt_user_ctx_t;

#define MT_MAX_TASKS 64
#define MT_KSTACK_SIZE 8192

typedef struct {
    ulime_proc_t *proc;
    mt_context_t ctx;
    mt_user_ctx_t user_ctx;
    u8 *kstack;
    int valid;
} mt_task_t;

typedef struct {
    mt_task_t tasks[MT_MAX_TASKS];
    int task_count;
    int current_idx;
    int initialized;
    scheduler_t *sched;
    ulime_t *ulime;
} mt_t;

extern void scheduler_context_switch(mt_context_t *old_ctx, mt_context_t *new_ctx);

void mt_init(mt_t *mt, scheduler_t *sched, ulime_t *ulime);
int mt_add_task(mt_t *mt, ulime_proc_t *proc);
void mt_start(mt_t *mt);
void mt_preempt(mt_t *mt, cpu_state_t *state);
void mt_yield(mt_t *mt);
int mt_task_count(mt_t *mt);
mt_task_t *mt_current_task(mt_t *mt);

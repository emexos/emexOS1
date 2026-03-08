#include "multitasking.h"
#include <kernel/arch/x86_64/gdt/gdt.h>
#include <kernel/communication/serial.h>

static int mt_find_next(mt_t *mt, int from_idx)
{
    if (mt->task_count == 0) return -1;

    int start = (from_idx + 1) % mt->task_count;
    int i = start;

    do {
        mt_task_t *t = &mt->tasks[i];
        if (t->valid && t->proc &&
            (t->proc->state == PROC_READY || t->proc->state == PROC_CREATED)) {
            return i;
        }
        i = (i + 1) % mt->task_count;
    } while (i != start);

    mt_task_t *t = &mt->tasks[start];
    if (t->valid && t->proc &&
        (t->proc->state == PROC_READY || t->proc->state == PROC_CREATED)) {
        return start;
    }

    return -1;
}

void mt_init(mt_t *mt, scheduler_t *sched, ulime_t *ulime)
{
    if (!mt) return;

    for (int i = 0; i < MT_MAX_TASKS; i++) {
        mt->tasks[i].proc = NULL;
        mt->tasks[i].valid = 0;
        mt->tasks[i].user_ctx.saved = 0;
    }

    mt->task_count = 0;
    mt->current_idx = -1;
    mt->initialized = 1;
    mt->sched = sched;
    mt->ulime = ulime;

    printf("[MT] multitasking initialized (max=%d quantum=%llu)\n",
           MT_MAX_TASKS,
           sched ? (unsigned long long)sched->quantum : 0ULL
    );
}

int mt_add_task(mt_t *mt, ulime_proc_t *proc)
{
    if (!mt || !proc) return -1;
    if (!mt->initialized) return -1;
    if (mt->task_count >= MT_MAX_TASKS) return -1;

    int idx = mt->task_count;
    mt_task_t *t = &mt->tasks[idx];

    t->proc = proc;
    t->valid = 1;
    t->user_ctx.saved = 0;
    t->user_ctx.cr3 = proc->pml4_phys;

    mt->task_count++;

    printf("[MT] task added: idx=%d pid=%llu entry=0x%llx\n",
           idx,
           (unsigned long long)proc->pid,
           (unsigned long long)proc->entry_point
    );

    return idx;
}

void mt_preempt(mt_t *mt, cpu_state_t *state)
{
    if (!mt || !mt->initialized || !state) return;
    if (mt->task_count < 2) return;

    // only switch if we interrupted user code
    if ((state->cs & 3) != 3) return;

    if (mt->sched) {
        mt->sched->ticks++;
        if (mt->sched->ticks < mt->sched->quantum) return;
        mt->sched->ticks = 0;
    }

    int old_idx = mt->current_idx;
    int new_idx = mt_find_next(mt, old_idx);

    if (new_idx < 0 || new_idx == old_idx) return;

    // save old process
    if (old_idx >= 0 && mt->tasks[old_idx].valid) {
        mt_task_t *old = &mt->tasks[old_idx];
        mt_user_ctx_t *uc = &old->user_ctx;

        uc->r15 = state->r15; uc->r14 = state->r14;
        uc->r13 = state->r13; uc->r12 = state->r12;
        uc->r11 = state->r11; uc->r10 = state->r10;
        uc->r9  = state->r9;  uc->r8  = state->r8;
        uc->rbp = state->rbp; uc->rdi = state->rdi;
        uc->rsi = state->rsi; uc->rdx = state->rdx;
        uc->rcx = state->rcx; uc->rbx = state->rbx;
        uc->rax = state->rax;

        uc->rip = state->rip;
        uc->cs = state->cs;
        uc->rflags = state->rflags;
        uc->rsp = state->rsp;
        uc->ss = state->ss;
        uc->cr3 = old->proc->pml4_phys;
        uc->saved = 1;

        if (old->proc->state == PROC_RUNNING)
            old->proc->state = PROC_READY
        ;
    }

    // load next process
    mt_task_t *next = &mt->tasks[new_idx];
    mt_user_ctx_t *nc = &next->user_ctx;

    if (nc->saved) {
        state->r15 = nc->r15; state->r14 = nc->r14;
        state->r13 = nc->r13; state->r12 = nc->r12;
        state->r11 = nc->r11; state->r10 = nc->r10;
        state->r9  = nc->r9;  state->r8  = nc->r8;
        state->rbp = nc->rbp; state->rdi = nc->rdi;
        state->rsi = nc->rsi; state->rdx = nc->rdx;
        state->rcx = nc->rcx; state->rbx = nc->rbx;
        state->rax = nc->rax;
        state->rip = nc->rip;
        state->cs  = nc->cs;
        state->rflags = nc->rflags;
        state->rsp = nc->rsp;
        state->ss = nc->ss;
    } else {
        // first run
        state->r15 = state->r14 = state->r13 = state->r12 = 0;
        state->r11 = state->r10 = state->r9  = state->r8  = 0;
        state->rbp = state->rdi = state->rsi = state->rdx = 0;
        state->rcx = state->rbx = state->rax = 0;

        state->rip = next->proc->entry_point;
        state->cs = (u64)(USER_CODE_SELECTOR | 3);
        state->rflags = 0x202; // IF=1
        state->rsp = (next->proc->stack_base + next->proc->stack_size - 16)& ~0xFULL;
        state->ss = (u64)(USER_DATA_SELECTOR | 3);
    }

    next->proc->state = PROC_RUNNING;
    mt->current_idx = new_idx;

    if (mt->ulime) mt->ulime->ptr_proc_curr = next->proc;

    __asm__ volatile(
    	"mov %0, %%cr3"
     	::
      	"r"(next->proc->pml4_phys)
       	:
        "memory"
    );
}

void mt_yield(mt_t *mt) {
    if (!mt || !mt->initialized) return;
    if (mt->sched) mt->sched->ticks = 0;
}

int mt_task_count(mt_t *mt) {
    if (!mt) return 0;
    return mt->task_count;
}

mt_task_t *mt_current_task(mt_t *mt) {
    if (!mt || mt->current_idx < 0) return NULL;
    return &mt->tasks[mt->current_idx];
}
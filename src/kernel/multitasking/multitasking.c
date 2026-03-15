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
        mt->tasks[i].kstack = NULL;
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
    t->kstack = (u8*)klime_create(mt->ulime->klime, MT_KSTACK_SIZE);

    mt->task_count++;

    printf("[MT] task added: idx=%d pid=%llu entry=0x%llx kstack=0x%llx\n",
           idx,
           (unsigned long long)proc->pid,
           (unsigned long long)proc->entry_point,
           (unsigned long long)t->kstack
    );

    return idx;
}

void mt_start(mt_t *mt)
{
    if (!mt || !mt->initialized || mt->task_count == 0) return;

    int idx = mt_find_next(mt, -1);
    if (idx < 0) {
        printf("[MT] mt_start: no ready task found\n");
        return;
    }

    mt_task_t *t  = &mt->tasks[idx];
    mt->current_idx = idx;

    t->proc->state = PROC_RUNNING;
    if (mt->ulime) mt->ulime->ptr_proc_curr = t->proc;
    if (t->kstack)
        gdt_set_kernel_stack(((u64)t->kstack + MT_KSTACK_SIZE) & ~0xFULL);

    u64 user_rip = t->proc->entry_point;
    u64 user_rsp;
    if (t->proc->entry_rsp != 0) {
        user_rsp = t->proc->entry_rsp;
    } else {
        user_rsp = (t->proc->stack_base + t->proc->stack_size - 16) & ~0xFULL;
    }

    printf("[MT] starting first task: idx=%d pid=%llu rip=0x%llx rsp=0x%llx\n",
           idx,
           (unsigned long long)t->proc->pid,
           (unsigned long long)user_rip,
           (unsigned long long)user_rsp
    );

    __asm__ volatile("mov %0, %%cr3" :: "r"(t->proc->pml4_phys) : "memory");
    __asm__ volatile(
        "cli\n"
        "subq $40, %%rsp\n"
        "movq %0, 32(%%rsp)\n"  // SS
        "movq %1, 24(%%rsp)\n"  // user RSP
        "movq %2, 16(%%rsp)\n"  // RFLAGS
        "movq %3,  8(%%rsp)\n"  // CS
        "movq %4,  0(%%rsp)\n"  // RIP
        // clears GPR
        "xor %%rax, %%rax\n"
        "xor %%rbx, %%rbx\n"
        "xor %%rcx, %%rcx\n"
        "xor %%rdx, %%rdx\n"
        "xor %%rsi, %%rsi\n"
        "xor %%rdi, %%rdi\n"
        "xor %%r8,  %%r8\n"
        "xor %%r9,  %%r9\n"
        "xor %%r10, %%r10\n"
        "xor %%r11, %%r11\n"
        "xor %%r12, %%r12\n"
        "xor %%r13, %%r13\n"
        "xor %%r14, %%r14\n"
        "xor %%r15, %%r15\n"
        "xor %%rbp, %%rbp\n"
        "iretq\n"
        :
        : "r"((u64)(USER_DATA_SELECTOR | 3)),
          "r"(user_rsp),
          "r"((u64)0x202),         // IF=1
          "r"((u64)(USER_CODE_SELECTOR | 3)),
          "r"(user_rip)
        : "memory"
    );
    __builtin_unreachable();
}

void mt_preempt(mt_t *mt, cpu_state_t *state)
{
    if (!mt || !mt->initialized || !state) return;
    if (mt->task_count < 2) return;
    // only preempt from usermode
    if ((state->cs & 3) != 3) {
        if (mt->current_idx < 0) return;
        mt_task_t *cur = &mt->tasks[mt->current_idx];
        if (!cur->valid || !cur->proc) return;
        if (cur->proc->state != PROC_BLOCKED) return;
        mt->current_idx = -1;
        return;
    }

    if (mt->current_idx >= 0) {
        mt_task_t *cur = &mt->tasks[mt->current_idx];
        if (cur->valid && cur->proc && cur->proc->state == PROC_BLOCKED) {
            if (mt->sched) mt->sched->ticks = mt->sched->quantum; // force switch
        }
    }
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
        uc->r9 = state->r9; uc->r8 = state->r8;
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

    if (next->kstack)
        gdt_set_kernel_stack(((u64)next->kstack + MT_KSTACK_SIZE) & ~0xFULL)
    ;

    //printf("[MT] switched %d -> %d\n", old_idx, new_idx);

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
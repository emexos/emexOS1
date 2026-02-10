#include "scheduler.h"
//#include "proc_manager.h"
#include <memory/main.h>
#include <kernel/mem/klime/klime.h>
#include <kernel/communication/serial.h>

// i literally only make this so usermode can work good...

/*#if ENABLE_ULIME
scheduler_t *scheduler = NULL;
//proc_manager_t *proc_mgr = NULL;
#endif*/


scheduler_t* scheduler_init(ulime_t *ulime, u64 quantum) {
    if (!ulime || !ulime->klime) return NULL;

    scheduler_t *s = (scheduler_t*)klime_create(ulime->klime, sizeof(scheduler_t));
    if (!s) return NULL;

    s->ulime = ulime;
    s->quantum = quantum;
    s->ticks = 0;

    return s;
}

void scheduler_set_quantum(scheduler_t *s, u64 quantum) {
    if (!s) return;
    s->quantum = quantum;
}

/*void sched_set_policy(sched_t *s, sched_policy_t policy) {
    if (!s) return;
    s->policy = policy;
}

void sched_set_timeslice(sched_t *s, u64 ticks) {
    if (!s) return;
    s->time_slice = ticks;
}

// pick next process based on policy
ulime_proc_t* sched_pick_next(sched_t *s) {
    if (!s || !s->ulime) return NULL;

    ulime_proc_t *current = s->ulime->ptr_proc_curr;
    ulime_proc_t *next = NULL;

    if (s->policy == SCHED_RR) {
        // round robin
        next = current ? current->next : s->ulime->ptr_proc_list;
        if (!next) next = s->ulime->ptr_proc_list;

        // find first ready
        ulime_proc_t *start = next;
        while (next && next->state != PROC_READY) {
            next = next->next;
            if (!next) next = s->ulime->ptr_proc_list;
            if (next == start) break;
        }

    } else {
        // find highest priority ready process
        ulime_proc_t *p = s->ulime->ptr_proc_list;
        u64 max_prio = 0;

        while (p) {
            if (p->state == PROC_READY && p->priority > max_prio) {
                max_prio = p->priority;
                next = p;
            }
            p = p->next;
        }
    }

    return (next && next->state == PROC_READY) ? next : NULL;
}

void sched_run(sched_t *s) {
    if (!s) return;

    ulime_proc_t *next = sched_pick_next(s);
    if (!next) return;

    ulime_proc_t *current = s->ulime->ptr_proc_curr;

    // switch if different
    if (next != current) {
        if (current && current->state == PROC_RUNNING) {
            current->state = PROC_READY;
        }

        next->state = PROC_RUNNING;
        s->ulime->ptr_proc_curr = next;
        s->ticks = 0;

        printf("[SCHED] switch to pid=%lu (%s)\n", next->pid, next->name);
    }
}
*/

void scheduler_tick(scheduler_t *s) {
    if (!s) return;

    s->ticks++;

    // prempt if quantum expired
    if (s->ticks >= s->quantum) {
        ulime_schedule(s->ulime);
        s->ticks = 0;
    }
}

void scheduler_yield(scheduler_t *s) {
    if (!s) return;

    ulime_schedule(s->ulime);
    s->ticks = 0;
}

/*void scheduler_stats(scheduler_t *s) {
    if (!s) return;

    printf("\n[SCHED] stats:\n");
    printf("  policy: %s\n", s->policy == SCHED_RR ? "RR" : "PRIO");
    printf("  time_slice: %lu\n", s->time_slice);
    printf("  ticks: %lu\n", s->ticks);

    if (s->ulime->ptr_proc_curr) {
        printf("  current: pid=%lu (%s)\n",
               s->ulime->ptr_proc_curr->pid,
               s->ulime->ptr_proc_curr->name);
    }
    printf("\n");
}
*/

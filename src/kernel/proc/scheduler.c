#include "scheduler.h"
#include <klib/string/string.h>
#include <theme/stdclrs.h>

#define QUANTUM 10

static sched_queue_t ready_q;
static proc_t *running;
static int enabled;
static u64 ticks;

static void queue_init(sched_queue_t *q) {
    q->head = q->tail = q->cnt = 0;
}

static int queue_empty(sched_queue_t *q) {
    return q->cnt == 0;
}

static void queue_push(sched_queue_t *q, proc_t *p) {
    if (q->cnt >= 32) return;
    q->queue[q->tail] = p;
    q->tail = (q->tail + 1) % 32;
    q->cnt++;
}

static proc_t *queue_pop(sched_queue_t *q) {
    if (queue_empty(q)) return NULL;
    proc_t *p = q->queue[q->head];
    q->head = (q->head + 1) % 32;
    q->cnt--;
    return p;
}

void sched_init(void) {
    print("[SCHED] ", GFX_GRAY_70);
    print("init Scheduler\n", GFX_ST_WHITE);
    queue_init(&ready_q);
    running = NULL;
    enabled = 0;
    ticks = 0;
}

void sched_enable(void) {
    enabled = 1;
}

void sched_disable(void) {
    enabled = 0;
}

int sched_is_enabled(void) {
    return enabled;
}

void sched_add(proc_t *p) {
    if (!p) return;
    process_set_state(p, PROC_READY);
    queue_push(&ready_q, p);
}

void sched_remove(proc_t *p) {
    if (!p) return;

    for (int i = 0; i < ready_q.cnt; i++) {
        int idx = (ready_q.head + i) % 32;
        if (ready_q.queue[idx] == p) {
            for (int j = i; j < ready_q.cnt - 1; j++) {
                int cur = (ready_q.head + j) % 32;
                int nxt = (ready_q.head + j + 1) % 32;
                ready_q.queue[cur] = ready_q.queue[nxt];
            }
            ready_q.cnt--;
            ready_q.tail = (ready_q.tail - 1 + 32) % 32;
            return;
        }
    }
}

void sched_tick(void) {
    if (!enabled) return;

    ticks++;

    if (ticks % QUANTUM == 0) {
        sched_yield();
    }
}

void sched_yield(void) {
    if (!enabled) return;

    if (running) {
        process_set_state(running, PROC_READY);
        queue_push(&ready_q, running);
    }

    running = queue_pop(&ready_q);
    if (running) {
        process_set_state(running, PROC_RUNNING);
    }
}

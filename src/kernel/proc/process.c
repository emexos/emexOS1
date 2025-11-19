#include "process.h"
#include <kernel/mem/kheap/kheap.h>
#include <klib/string/string.h>
#include <theme/stdclrs.h>

#define STACK_SIZE 8192

static proc_t *head;
static proc_t *current;
static u64 next_pid;

void process_init(void) {
    print("[PROC] ", GFX_GRAY_70);
    print("init Process manager\n", GFX_ST_WHITE);
    head = NULL;
    current = NULL;
    next_pid = 1;
}

proc_t *process_create(const char *name, u64 entry) {
    proc_t *p = (proc_t *)kmalloc(sizeof(proc_t));
    if (!p) return NULL;

    u64 stk = (u64)kmalloc(STACK_SIZE);
    if (!stk) {
        kfree((u64 *)p);
        return NULL;
    }

    p->pid = next_pid++;
    p->state = PROC_READY;
    p->stack_base = stk;
    p->stack_ptr = stk + STACK_SIZE;
    p->entry = entry;
    p->next = head;

    int i = 0;
    while (name[i] && i < 63) {
        p->name[i] = name[i];
        i++;
    }
    p->name[i] = '\0';

    head = p;
    return p;
}

void process_destroy(proc_t *p) {
    if (!p) return;

    proc_t *cur = head, *prev = NULL;

    while (cur) {
        if (cur == p) {
            if (prev) prev->next = cur->next;
            else head = cur->next;

            if (current == p) current = NULL;

            kfree((u64 *)p->stack_base);
            kfree((u64 *)p);
            return;
        }
        prev = cur;
        cur = cur->next;
    }
}

proc_t *process_get_current(void) {
    return current;
}

void process_set_state(proc_t *p, proc_state_t state) {
    if (p) p->state = state;
}

#include "scheduler.h"
#include "process.h"

static int enabled = 0;

// Initialize scheduler
void scheduler_init(void) {
    enabled = 0;

    // TODO: setup queues
    // TODO: setup timer pit/ in future: tsc
}

// Called by timer interrupt
void scheduler_tick(void)
{
    if (!enabled) {
        return;
    }

    // TODO: scheduling
}

// Check if scheduler is enabled
int scheduler_is_enabled(void) {
    return enabled;
}

// in future functions like:
// - scheduler_start()
// - scheduler_add_process()
// - scheduler_switch()

//

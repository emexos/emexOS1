#include "process.h"

// process table (for future use)
static process_t *process_list = NULL;
static u64 next_pid = 1;


void process_init(void) {
    process_list = NULL;
    next_pid = 1;

    //
    // here should be scheduler
}

// in future functions like:
// - process_create()
// - process_destroy()
// - process_switch()
// can be used for the userspace for every window (app)
// or background apps

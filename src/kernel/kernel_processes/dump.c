#include "loader.h"

#include <kernel/communication/serial.h>
#include <string/string.h>

// only reason that this is in a extra file is cuz i want to make it more advanced... but not yet...

void dump_kprocesses(void) {
    printf("[KPROC] table (%d/%d)\n", kproc_cnt, KPROC_MAX);
    for (int i = 0; i < kproc_cnt; i++) {
        kproc_t *p = kproc_table[i];
        if (!p) continue;
        printf("  [%d] %-20s  (%-8s)  (%d)  flags=0x%02X\n",
               i, p->name, state_str(p->state), p->priority, p->flags);
    }
}

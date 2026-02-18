// src/kernel/console/functions/whoami.c
#include <kernel/console/console.h>

FHDR(cmd_whoami) {
    (void)s;

    cprintf(uci_get_user_name(), GFX_WHITE);
    cprintf("\n", GFX_WHITE);
}

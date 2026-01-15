// src/kernel/console/functions/whoami.c
#include <kernel/console/console.h>

FHDR(cmd_whoami) {
    (void)s;

    print(user_config_get_user_name(), GFX_WHITE);
    print("\n", GFX_WHITE);
}

#include "init.h"
#include "emxrc.h"

#include <unistd.h>
#include <stdio.h>
#include <unistd.h>

//-////////////////////////////////////////-//
//-//                                    //-//
//-//               INITD                //-//
//-//     the init demon for emexOS      //-//
//-//                                    //-//
//-////////////////////////////////////////-//

#define __INIT_SYSTEM_VER 1
#define __INIT_D "initd"

int main(void) {
    static emxrc_t rc;
    #define __INIT_D_BRACKETS "[" __INIT_D "]"

    if (emxrc_parse(EMXRC_PATH, &rc) != 0) {
        fprintf(stderr, __INIT_D_BRACKETS " .emxrc not found, using defaults!\n");
        char *const argv[] = { (char *)LOGINLOCATE, (char *)0 };
        char *const envp[] = { (char *)0 };
        execve(LOGINLOCATE, argv, envp);
        goto error;
    }

    emxrc_run(&rc);

error:
    fprintf(stderr, __INIT_D_BRACKETS " all execs failed, halting\n");
    for (;;) __asm__ volatile("pause");
    return 0;
}
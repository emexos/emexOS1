#include "init.h"
#include "emxrc.h"

#include <unistd.h>
#include <stdio.h>
#include <unistd.h>

int main(void) {
    static emxrc_t rc;

    if (emxrc_parse(EMXRC_PATH, &rc) != 0) {
        fprintf(stderr, "[init] .emxrc not found, using defaults!\n");
        char *const argv[] = { (char *)LOGINLOCATE, (char *)0 };
        char *const envp[] = { (char *)0 };
        execve(LOGINLOCATE, argv, envp);
        goto error;
    }

    emxrc_run(&rc);

error:
    fprintf(stderr, "[init] all execs failed, halting\n");
    for (;;) __asm__ volatile("pause");
    return 0;
}
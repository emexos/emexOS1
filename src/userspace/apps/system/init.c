#include "init.h"

#include <unistd.h>
#include <stdio.h>
//#include <string.h>
#include <sys/types.h>

const char* handlers[] = EMRHANDLERS;

int main(void)
{
    printf("[ESH] starting...\n");
    printf("[ESH] launching programms:\n");

    // other initializations...


    emx_shell: {
        printf("    - launching shell: %s\n", EMX_SHELL);
        // launch the shell as a new process via execve
        char *const argv[] = { (char *)EMX_SHELL, (char *)0 };
        char *const envp[] = { (char *)0 };
        execve(EMX_SHELL, argv, envp);
        goto error;
    }

error:
    printf("[ESH] ERROR: failed to launch %s\n", EMX_SHELL);
    for (;;) __asm__ volatile("pause");
    return 0;
}

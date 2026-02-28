#include "init.h"

#include <unistd.h>
#include <stdio.h>
//#include <string.h>
#include <sys/types.h>

const char* handlers[] = EMRHANDLERS;

int main(void)
{
    //printf("[ESH] starting...\n");
    //printf("[ESH] launching programms:\n");

    // other initializations...
    rc: {
        //TODO:
        // create file .emxrc
        // content: exec emx_window_system
        // dir: /emr/system/
    };

    //clear

    login: {
        //printf("     launching: %s\n", LOGINLOCATE);
        char *const argv[] = { (char *)LOGINLOCATE, (char *)0 };
        char *const envp[] = { (char *)0 };
        execve( LOGINLOCATE, argv, envp);
        //
        // load .emxrc and look for login.elf location
        //
    };

    emx_shell: {
        //printf("     launching: %s\n", EMX_SHELL);
        //printf(" \n");
        printf(":[%s]\n", EMX_SHELL);
        // launch the shell as a new process via execve
        char *const argv[] = { (char *)EMX_SHELL, (char *)0 };
        char *const envp[] = { (char *)0 };
        execve(EMX_SHELL, argv, envp);

        //
        // load .emxrc and switch to ws
        // and initialise de + wm
        //
        goto error;
    };
    goto error;
error:
    printf("[ESH] ERROR: failed to launch %s\n", EMX_SHELL);
    for (;;) __asm__ volatile("pause");
    return 0;
}

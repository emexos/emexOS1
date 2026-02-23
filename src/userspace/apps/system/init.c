#include "init.h"
#include <unistd.h>
//#include <stdio.h>
#include <string.h>

const char* handlers[] = EMRHANDLERS;

int main(void)
{
    const char emr_start[] = "[emr_system] starting...\n";//Hello from Userspace my friends :)";
    //const char emr_info[] = "[emr_system] emr_system is the base init system for emexOS";

    //printf("test printf\n\n");
    write(STDOUT_FILENO, emr_start, sizeof(emr_start) - 1);

    /*size_t handlers_count = sizeof(handlers) / sizeof(handlers[0]);
    char buffer[128];

    for (size_t i = 0; i < handlers_count; i++)
    {
        strncpy(buffer, handlers[i], sizeof(buffer)-1);
        buffer[sizeof(buffer)-1] = 0;
        write(STDOUT_FILENO, buffer, strlen(buffer));
        write(STDOUT_FILENO, "\n", 1);
    }*/

    for (;;) __asm__ volatile("pause");
    return 0;
}

#ifndef EXCEPTION_HANDLER_H
#define EXCEPTION_HANDLER_H

#include <types.h>
#include <kernel/cpu/idt.h>

extern const char* exception_messages[32];


void exception_handler(cpu_state_t *state);

#endif

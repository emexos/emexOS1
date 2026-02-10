#ifndef EXCEPTION_HANDLER_H
#define EXCEPTION_HANDLER_H

#include <types.h>
#include <kernel/arch/x86_64/idt/idt.h>
//#include <kernel/cpu/cpu.h>

extern const char* exception_messages[32];


void exception_handler(cpu_state_t *state);

#endif

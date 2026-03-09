#ifndef PANIC_H
#define PANIC_H

#include <types.h>
#include <kernel/arch/x86_64/idt/idt.h>

#define PANIC_SHOWSWITCH 1

// Kernel panic
__attribute__((noreturn)) void panic(const char *message);
__attribute__((noreturn)) void panic_exception(cpu_state_t *state, const char *message);

#endif

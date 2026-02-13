#ifndef SYSCALL_INIT_H
#define SYSCALL_INIT_H

#include <types.h>

// initialize syscall mechanism
void syscall_arch_init(void);

// syscall entry (assembly)
extern void syscall_entry(void);

#endif

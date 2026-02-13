#ifndef SYSCALL_H
#define SYSCALL_H
#include "ulime.h"
//#include "calls.h"

//void _init_syscalls(ulime_t *ulime);
void _init_syscalls_table(ulime_t *ulime);

u64 syscall_handler(u64 syscall_num, u64 arg1, u64 arg2, u64 arg3);

void JumpToUserspace(ulime_proc_t *proc);

#endif

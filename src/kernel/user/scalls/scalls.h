#pragma once

#include <types.h>
#include <kernel/user/ulime.h>

u64 scall_mq_open(ulime_proc_t *proc, u64 name_ptr, u64 oflag, u64 mode);
u64 scall_mq_unlink(ulime_proc_t *proc, u64 name_ptr, u64 arg2, u64 arg3);
u64 scall_mq_send(ulime_proc_t *proc, u64 mqid, u64 buf, u64 size);
u64 scall_mq_recv(ulime_proc_t *proc, u64 mqid, u64 buf, u64 size);

u64 scall_mmap(ulime_proc_t *proc, u64 addr, u64 length, u64 shm_id);
u64 scall_munmap(ulime_proc_t *proc, u64 addr, u64 length, u64 arg3);
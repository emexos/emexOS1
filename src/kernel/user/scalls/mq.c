#include "scalls.h"
#include <kernel/multitasking/ipc/ipc.h>
#include <kernel/user/ptrs.h>

u64 scall_mq_open(ulime_proc_t *proc, u64 name_ptr, u64 oflag, u64 mode)
{
    (void)oflag;
    (void)mode;

    if (!is_valid_user_ptr(name_ptr)) return (u64)-1;

    open_inbox(proc->ulime, proc);
    return (u64)proc->pid;
}

u64 scall_mq_unlink(ulime_proc_t *proc, u64 name_ptr, u64 arg2, u64 arg3)
{
    (void)name_ptr;
    (void)arg2;
    (void)arg3;

    destroy_endpoint(proc->ulime, proc);
    return 0;
}

u64 scall_mq_send(ulime_proc_t *proc, u64 mqid, u64 buf, u64 size) {
	int r = send_msg(proc->ulime, proc, mqid, (const void *)buf, (u32)size);

    if (!is_valid_user_ptr(buf)) return (u64)-1;
    if (size == 0 || size > IPC_MAX_MSG_SIZE) return (u64)-1;

    return r == 0 ? 0 : (u64)-1;
}

u64 scall_mq_recv(ulime_proc_t *proc, u64 mqid, u64 buf, u64 size)
{
    (void)mqid;

    u64 sender = 0;
    int got = receive_async_msg(proc->ulime, proc, (void *)buf, (u32)size, &sender);

    if (!is_valid_user_ptr(buf)) return (u64)-1;
    if (size == 0) return (u64)-1;
    if (got < 0) {
        receive_msg(proc->ulime, proc, (void *)buf, (u32)size, &sender);
        got = (int)size;
    }

    return (u64)got;
}
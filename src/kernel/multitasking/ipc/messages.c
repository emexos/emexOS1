#include "ipc.h"
#include <memory/main.h>
#include <kernel/communication/serial.h>

extern klime_t *klime;

#define MAX_MESSAGES 256

static ipc_endpoint_t *endpoints[IPC_MAX_PROCS];

void ipc_messages_init(void) {
    for (int i = 0; i < IPC_MAX_PROCS; i++) endpoints[i] = NULL;
}

static ipc_endpoint_t *get_ep(u64 pid) {
    if (pid >= IPC_MAX_PROCS) return NULL;
    return endpoints[pid];
}

static ulime_proc_t *find_proc(ulime_t *ulime, u64 pid) {
    ulime_proc_t *p = ulime->ptr_proc_list;
    while (p) {
        if (p->pid == pid) return p;
        p = p->next;
    }
    return NULL;
}

static void wake_waiters(ipc_endpoint_t *ep) {
    for (int i = 0; i < ep->waiter_count; i++) {
        if (ep->waiters[i] && ep->waiters[i]->state == PROC_BLOCKED)
            ep->waiters[i]->state = PROC_READY;
    }
    ep->waiter_count = 0;
}

void open_inbox(ulime_t *ulime, ulime_proc_t *proc)
{
    u64 pid = proc->pid;
    if (pid >= IPC_MAX_PROCS) return;

    if (endpoints[pid]) {
        endpoints[pid]->is_open = 1;
        return;
    }

    ipc_endpoint_t *ep = (ipc_endpoint_t *)klime_create(ulime->klime, sizeof(ipc_endpoint_t));
    if (!ep) return;

    ep->is_open = 1;
    ep->count = 0;
    ep->waiter_count = 0;
    for (int i = 0; i < IPC_MAX_MESSAGES; i++) ep->messages[i] = NULL;
    for (int i = 0; i < IPC_MAX_WAITERS;  i++) ep->waiters[i] = NULL;

    endpoints[pid] = ep;
}

void close_inbox(ulime_proc_t *proc) {
    ipc_endpoint_t *ep = get_ep(proc->pid);
    if (ep) ep->is_open = 0;
}

void destroy_endpoint(ulime_t *ulime, ulime_proc_t *proc)
{
    u64 pid = proc->pid;
    ipc_endpoint_t *ep = get_ep(pid);
    if (!ep) return;

    close_inbox(proc);

    for (int i = 0; i < ep->count; i++) {
        if (ep->messages[i]) {
            klime_free(ulime->klime, (u64 *)ep->messages[i]);
            ep->messages[i] = NULL;
        }
    }
    ep->count = 0;

    wake_waiters(ep);

    klime_free(ulime->klime, (u64 *)ep);
    endpoints[pid] = NULL;
}

int send_msg(
	ulime_t *ulime, ulime_proc_t *sender, u64 receiver_id,
    const void *data, u32 size
) {
    if (!data || size == 0 || size > IPC_MAX_MSG_SIZE) return -1;

    ipc_endpoint_t *ep = get_ep(receiver_id);
    if (!ep || !ep->is_open) return -1;

    while (ep->count >= IPC_MAX_MESSAGES) {
        if (ep->waiter_count < IPC_MAX_WAITERS)
            ep->waiters[ep->waiter_count++] = sender;
        sender->state = PROC_BLOCKED;

        while (sender->state == PROC_BLOCKED)
            __asm__ volatile("pause");
    }

    ipc_msg_t *msg = (ipc_msg_t *)klime_create(ulime->klime, sizeof(ipc_msg_t));
    if (!msg) return -1;

    msg->sender_id = sender->pid;
    msg->receiver_id = receiver_id;
    msg->size = size;
    msg->used = 1;
    memcpy(msg->data, data, size);

    ep->messages[ep->count++] = msg;

    ulime_proc_t *recv_proc = find_proc(ulime, receiver_id);
    if (recv_proc && recv_proc->state == PROC_BLOCKED)
        recv_proc->state = PROC_READY;

    return 0;
}

int receive_async_msg(ulime_t *ulime, ulime_proc_t *proc, void *buf, u32 buf_size, u64 *out_sender)
{
    ipc_endpoint_t *ep = get_ep(proc->pid);
    if (!ep || ep->count <= 0) return -1;

    ipc_msg_t *msg = ep->messages[0];
    u32 copy_size = msg->size < buf_size ? msg->size : buf_size;

    memcpy(buf, msg->data, copy_size);
    if (out_sender) *out_sender = msg->sender_id;

    ep->count--;
    for (int i = 0; i < ep->count; i++)
        ep->messages[i] = ep->messages[i + 1];
    ep->messages[ep->count] = NULL;

    klime_free(ulime->klime, (u64 *)msg);

    if (ep->waiter_count > 0)
        wake_waiters(ep);

    return (int)copy_size;
}

void receive_msg(ulime_t *ulime, ulime_proc_t *proc, void *buf, u32 buf_size, u64 *out_sender) {
    while (receive_async_msg(ulime, proc, buf, buf_size, out_sender) < 0) {
        proc->state = PROC_BLOCKED;
        while (proc->state == PROC_BLOCKED)
            __asm__ volatile("pause");
    }
}
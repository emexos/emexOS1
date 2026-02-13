#include <kernel/user/user.h>
#include <kernel/communication/serial.h>
#include <string/string.h>
#include <string/log.h>

#include <theme/doccr.h>
#include <config/system.h>

/*
// create a process for the kernel
// if proc: kernel == quit the system stops/halt/crashes idk
void create_kernel_process(ulime_t *ulime, proc_manager_t *proc_mgr)
{
    ulime_proc_t *kernel_proc = ulime_proc_create(
        ulime,
        (u8*)"kernel",
        (u64)kmain
    );

    kernel_proc->priority = 255;
    kernel_proc->state = PROC_READY;
    kernel_proc->is_critical = 1;

    // register process manager
    proc_add(proc_mgr, kernel_proc);

    log("[KPROC]", "Kernel process created\n", 0);
}
*/

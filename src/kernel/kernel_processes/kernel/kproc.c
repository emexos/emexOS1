#include "gen.h"

/*#if ENABLE_ULIME
extern proc_manager_t *proc_mgr;
extern ulime_t *ulime;
#endif
*/


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

void kproc(void ) {

    #if ENABLE_ULIME && RUNTESTS // set to 1 to enable test
    //BOOTUP_PRINT("test RUNTESTS block\n", white());
        if (proc_mgr){
            ulime_proc_t *p1 = proc_create_proc(proc_mgr, (u8*)KERNELPROC, KERNELSPACE, KERNELPRIORITY);
            ulime_proc_t *p2 = proc_create_proc(proc_mgr, (u8*)"__rt", 0x40001000, 2); // run tests

            log("[KPROC]", "creating test processes\n", d);
            if (p1) log("[KPROC]", "created kernel\n", d);
            if (p2) log("[KPROC]", "created __rt\n", d);

            //proc_list_procs(proc_mgr);

            //log("[TEST_CODE]", "loading test code:\n", d);
            //ulime_load_program(p4, test_code, sizeof(test_code));
            //log("[TEST_CODE]", "test code loaded successfully\n", d);
            //JumpToUserspace(p4);
            log("[KPROC]", "Kernel processes created\n", d);
        }
        //proc_list_procs(proc_mgr);
    #endif

};
//void kproc(void);

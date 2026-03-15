#include "gen.h"

/*#if ENABLE_ULIME
extern proc_manager_t *proc_mgr;
extern ulime_t *ulime;
#endif
*/

static int kernelprocesses2_init(kproc_t *self) {
    (void)self;
    printf("[KERNELPROCESSES2] initialising...\n");
    kproc();
    return KPROC_EFINE;
}

static int kernelprocesses2_tick(kproc_t *self) {
    (void)self;
    return KPROC_EFINE;
}

static void kernelprocesses2_fini(kproc_t *self) {
    (void)self;
}

//static u8 test_loop[] = { 0xEB, 0xFE }; // jmp $

void kproc(void ) {

    #if ENABLE_ULIME && RUNTESTS // set to 1 to enable test
    //BOOTUP_PRINT("test RUNTESTS block\n", white());
        if (proc_mgr && mt) {
            ulime_proc_t *p1 = proc_create_proc(proc_mgr, (u8*)KERNELPROC, KERNELSPACE, KERNELPRIORITY);
            ulime_proc_t *p2 = proc_create_proc(proc_mgr, (u8*)"__rt", KERNELSPACE + 0x00001000, 2); // run tests

            log("[KPROC]", "creating kernel processes...\n", d);
            if (p1) log("[KPROC]", "created kernel\n", d);
            if (p2) log("[KPROC]", "created __rt\n", d);

            //proc_list_procs(proc_mgr);

            //log("[TEST_CODE]", "loading test code:\n", d);
            //ulime_load_program(p4, test_code, sizeof(test_code));
            //log("[TEST_CODE]", "test code loaded successfully\n", d);
            //JumpToUserspace(p4);
            log("[KPROC]", "Kernel processes created.\n", d);
        }
        //proc_list_procs(proc_mgr);
    #endif

}
//void kproc(void);
//
kproc_t kernelprocesses2_proc = {
    .name     = "kproc_kernel",
    .state    = KPROC_STATE_READY,
    .flags    = KPROC_FLAG_EARLY | KPROC_FLAG_CRITICAL | KPROC_FLAG_PERMANENT,
    .priority = 255,
    .init     = kernelprocesses2_init,
    .tick     = kernelprocesses2_tick,
    .fini     = kernelprocesses2_fini,
};

void init_kernelprocesses2(void) {
    kproc_register_and_start(&kernelprocesses2_proc);
}

#include <kernel/user/user.h>
#include <kernel/user/ulime.h>
#include <kernel/proc/proc_manager.h>
#include <kernel/exec/elf/loader.h>
#include <kernel/mem/paging/paging.h>
#include <kernel/communication/serial.h>
#include <kernel/arch/x86_64/gdt/gdt.h>
#include <kernel/include/reqs.h>
#include <string/string.h>
#include <string/log.h>

#include <theme/doccr.h>
#include <config/system.h>
#include <config/user.h>

#include <limine/limine.h>

#if ENABLE_ULIME
extern proc_manager_t *proc_mgr;
extern ulime_t *ulime;
#endif


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
        if (
            proc_mgr &&
            ulime &&
            module_request.response &&
            module_request.response->module_count
        > 0){

            ulime_proc_t *p4 = proc_create_proc(proc_mgr, (u8*)USERPROC, USERSPACE, USERPRIORITY);
            if (p4) log("[KPROC]", "created user process\n", d);
            #define USERPROGRAMMSPACE 0x40004200
            #define USERPROGRAMMPRIORITY 1

            struct limine_module_response *mod_resp = module_request.response;
            struct limine_file *hello_mod = NULL;

            for (u64 i = 0; i < mod_resp->module_count; i++) {
                const char *path = mod_resp->modules[i]->path;
                if (str_contains((char*)path, "hello.elf")) {
                    hello_mod = mod_resp->modules[i];
                    break;
                }
            }

            if (hello_mod)
            {
                log("[ELF]", "found hello.elf module\n", d);

                ulime_proc_t *proc = proc_create_proc(proc_mgr, (u8*)"hello", USERPROGRAMMSPACE, USERPROGRAMMPRIORITY);
                if (proc)
                {
                    u8 *elf_data = (u8*)hello_mod->address;
                    u64 elf_size = hello_mod->size;

                    if (elf_load(proc, elf_data, elf_size) == 0) {
                        ulime->ptr_proc_curr = proc;
                        log("[ELF]", "jumping to userspace\n", d);
                        u64 user_rsp = (proc->stack_base + proc->stack_size - 16) & ~0xFULL;

                        verify_page_permissions(proc->ulime->hpr, proc->entry_point, "Code page");
                        verify_page_permissions(proc->ulime->hpr, user_rsp, "Stack page");
                        verify_gdt_setup();
                        //JumpToUserspace(proc);
                    }
                }
            } else {
                log("[ELF]", "hello.elf not found in ramfs\n", error);
            }
        }
        proc_list_procs(proc_mgr);
    #endif

};
//void kproc(void);

#include "gen.h"
#define USERPROGRAMMSPACE 0x40004200
#define USERPROGRAMMPRIORITY 1

void uproc(void ) {
    #if ENABLE_ULIME && RUNTESTS // set to 1 to enable test
        if (
            proc_mgr &&
            ulime &&
            module_request.response &&
            module_request.response->module_count
        > 0){

            ulime_proc_t *p4 = proc_create_proc(proc_mgr, (u8*)USERPROC, USERSPACE, USERPRIORITY);
            if (p4) log("[KPROC]", "created user process\n", d);

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
        //proc_list_procs(proc_mgr);
    #endif

};

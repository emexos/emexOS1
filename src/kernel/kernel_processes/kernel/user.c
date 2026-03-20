#include "gen.h"
#include "../shells/shells.h"
#include <kernel/packages/emex/emex.h>

// path to the init app in the VFS (loaded from initrd.cpio)
#define SYSTEMLOCATE "/emr/system/system.emx"
//mod
//#define LOGINLOCATE "/emr/bin/login.elf"

void uproc(void) {
    #if ENABLE_ULIME
        if (!proc_mgr || !ulime) {
            log("[INIT]", "proc_mgr or ulime not ready\n", error);
            return;
        }

        log("[INIT]", "loading " SYSTEMLOCATE "...\n", d);

        ulime_proc_t *init_proc = NULL;
        int result = emex_launch_app(SYSTEMLOCATE, &init_proc);

        if (result != EMEX_OK || !init_proc) {
            log("[INIT]", "failed to load " SYSTEMLOCATE "\n", error);
            log("[INIT]", "system cannot continue without init\n", error);
            //while (1) __asm__ volatile("cli; hlt");
            // hcf();
            return;
        }

        log("[INIT]", "jumping to userspace\n", success);

        //first clear screen
        clear(0xff000000);

        dump_kprocesses();
        proc_list_procs(proc_mgr);
        ulime->ptr_proc_curr = init_proc;

        log("[INIT]", "jumping to init_proc\n", d);

        #if JUMPTOUSER == 1

        	if (mt) {
	            mt_add_task(mt, init_proc); // register init with mt
	            mt_start(mt); // and launch
				//never returns
	        }
            JumpToUserspace(init_proc); // fallback if mt is not up

        #endif

        // if JumpToUserspace fails
        emergency_shell();

        // if emergency_shell fails
        recovery_shell();

        // if recovery_shell fails
        __builtin_unreachable();

        //proc_list_procs(proc_mgr);
    #endif
}

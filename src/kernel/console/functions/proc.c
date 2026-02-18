#include <kernel/console/console.h>

FHDR(cmd_ps)
{
#if ENABLE_ULIME && RUNTESTS
    if (!proc_mgr)
    {
        cprintf("proc_mgr not initialized\n", white());
        return;
    }

    proc_list_procs(proc_mgr);
#endif
}

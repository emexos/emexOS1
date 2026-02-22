#ifndef GEN_HEADER
#define GEN_HEADER


#include "../loader.h"


#include <kernel/user/user.h>
#include <kernel/user/ulime.h>
#include <kernel/proc/proc_manager.h>
#include <kernel/packages/elf/loader.h>
#include <kernel/mem/paging/paging.h>
#include <kernel/communication/serial.h>
#include <kernel/arch/x86_64/gdt/gdt.h>
#include <kernel/include/reqs.h>
#include <string/string.h>

#include <theme/doccr.h>
#include <config/system.h>
#include <config/user.h>

#include <limine/limine.h>

#if ENABLE_ULIME
extern proc_manager_t *proc_mgr;
extern ulime_t *ulime;
#endif

void kproc(void);
void init_kernelprocesses2(void);
void uproc(void);
void genprocs(void);

#endif

#ifndef GENERALINIT_H // init.h
#define GENERALINIT_H

#include <limine/limine.h>
//#include <config/paths.h>
#include <kernel/file_systems/vfs/vfs.h>
#include <kernel/communication/serial.h>
#include <kernel/modules/limine.h>
#include <kernel/graph/theme.h>
#include <theme/doccr.h>

#include <config/user.h>
#include <config/system.h>

// Load all Limine modules to VFS
void dualslotvalidating(void);
void keymaps_load(void);
void logos_load(void);
void users_load(void);

#endif

#ifndef SLOTH
#define SLOTH

#include <kernel/file_systems/vfs/vfs.h>
#include <kernel/cpu/poweroff.h>

#define SLOT_PATH "/boot/activeslot.cfg"

int writeslot(char slot);
char readslot(void);
void dualslotvalidating(void);

#endif

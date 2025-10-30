#ifndef CMOS_H
#define CMOS_H

#include "../../../shared/types.h"

void outb(u16 port, u8 value);
char inb(u16 port);
USHORT GetCMOSMem(void);
void GetCMOSDate(void);

#endif

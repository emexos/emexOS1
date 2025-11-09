#ifndef PORTS_H
#define PORTS_H

#include <types.h>

// Port I/O
void outb(u16 port, u8 val);
u8 inb(u16 port);
void outw(u16 port, u16 val);
u16 inw(u16 port);
void outl(u16 port, u32 val);
u32 inl(u16 port);

// I/O wait/delay
void io_wait(void);

#endif

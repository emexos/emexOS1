#ifndef CMOS_H
#define CMOS_H

#include <types.h>

// time
typedef struct {
    u8 second;
    u8 minute;
    u8 hour;
    u8 day;
    u8 month;
    u8 year;     // 2-digit == 2025 ->25
} cmos_time_t;

void cmos_read_time(cmos_time_t *time);
u64 cmos_get_unix_timestamp(void);
void GetCMOSTime(void);
/*void outb(u16 port, u8 value);
char inb(u16 port);*/
USHORT GetCMOSMem(void);
void GetCMOSDate(void);

#endif

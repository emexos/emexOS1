#include "cmos.h"
#include <kernel/include/ports.h>
#include <klib/graphics/graphics.h>

/*
 * ----------------------------------------------------------------------------------
 * NOTE: THIS IS NOT MY CODE!
 * This code is from: https://github.com/ArTicZera/NovaOS/blob/main/Hardware/cmos.c
 * *MODIFIED
 * ----------------------------------------------------------------------------------
 */

USHORT GetCMOSMem()
{
    //Sends low memory in KB
    outb(0x70, 0x30);
    u8 low = inb(0x71);

    //Sends high memory in KB
    outb(0x70, 0x31);
    u8 high = inb(0x71);

    //Combine the lowest with the highest
    USHORT total = low | high << 8;

    return total;
}

void GetCMOSDate()
{
    //Access CMOS to get date
    outb(0x70, 0x0A);
    while(inb(0x71) & 0x80);

    //Gets the day
    outb(0x70, 0x07);
    u8 day = inb(0x71);

    //Gets the month
    outb(0x70, 0x08);
    u8 month = inb(0x71);

    //Gets the year
    outb(0x70, 0x09);
    u8 year = inb(0x71);

    //Converts BCD to Decimal
    day   = ((day   / 16) * 10) + (day   & 0x0F);
    month = ((month / 16) * 10) + (month & 0x0F);
    year  = ((year  / 16) * 10) + (year  & 0x0F);

    printInt(month, GFX_WHITE);
    print("/", GFX_WHITE);

    printInt(day, GFX_WHITE);
    print("/", GFX_WHITE);

    printInt(year, GFX_WHITE);
}

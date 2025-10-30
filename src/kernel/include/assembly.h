#ifndef ASSEMBLY_H
#define ASSEMBLY_H

#include "../../../shared/types.h"
#include "../../../third_party/limine/limine.h"


// limine reqs
__attribute__((used, section(".requests")))
static volatile LIMINE_BASE_REVISION(3);

__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

// halt
static inline void hcf(void)
{
    for (;;) {
        __asm__ volatile ("hlt");
    }
}

// simple delay
static inline void delay(u32 count)
{
    for (volatile u32 i = 0; i < count * 10000000; i++)
    {
        __asm__ volatile ("nop");
    }
    //maybe i add apic and timer later
}


#endif

#ifndef ASSEMBLY_H
#define ASSEMBLY_H

#include <types.h>
/*#include <limine/limine.h>

// limine reqs
__attribute__((used, section(".requests")))
static volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(3);

// framebuffer
__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0,
    .response = NULL
    };*/
    //** moved to reqs.c/.h **

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

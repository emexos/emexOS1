#include "idt.h"
#include <kernel/arch/x86_64/exceptions/isr.h>
#include <kernel/arch/x86_64/exceptions/irq.h>
#include <memory/main.h>
#include <string/string.h>
#include <theme/stdclrs.h>
#include <kernel/graph/theme.h>
#include <theme/doccr.h>

static idt_entry_t idt[IDT_ENTRIES];
static idt_ptr_t idt_ptr;

extern void idt_flush(u64);

void idt_set_gate(u8 num, u64 handler, u8 flags)
{
    idt[num].offset_low = handler & 0xFFFF;
    idt[num].selector = 0x08; // kernel Code Segment
    idt[num].ist = 0;
    idt[num].flags = flags;
    idt[num].offset_mid = (handler >> 16) & 0xFFFF;
    idt[num].offset_high = (handler >> 32) & 0xFFFFFFFF;
    idt[num].reserved = 0;
}

void idt_set_gate_ist(u8 num, u64 handler, u8 flags, u8 ist)
{
    idt_set_gate(num, handler, flags);
    idt[num].ist = ist & 0x07;
}

void idt_load(void)
{
    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base = (u64)&idt;

    idt_flush((u64)&idt_ptr);
}

void idt_init(void)
{
    log("[IDT]", "Init interrupts\n", d);
    memset(&idt, 0, sizeof(idt)); // clear idt
    isr_install();
    irq_install();
    idt_load();
}

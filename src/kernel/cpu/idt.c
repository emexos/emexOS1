#include "idt.h"
#include <kernel/exceptions/isr.h>
#include <kernel/exceptions/irq.h>
#include <klib/memory/main.h>
#include <klib/string/string.h>
#include <theme/stdclrs.h>

static idt_entry_t idt[IDT_ENTRIES];
static idt_ptr_t idt_ptr;

extern void idt_flush(u64);

void idt_set_gate(u8 num, u64 handler, u8 flags)
{
    idt[num].offset_low = handler & 0xFFFF;
    idt[num].selector = 0x08; // Kernel Code Segment
    idt[num].ist = 0;
    idt[num].flags = flags;
    idt[num].offset_mid = (handler >> 16) & 0xFFFF;
    idt[num].offset_high = (handler >> 32) & 0xFFFFFFFF;
    idt[num].reserved = 0;
}

void idt_load(void)
{
    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base = (u64)&idt;

    idt_flush((u64)&idt_ptr);
}

void idt_init(void)
{
    print("[IDT] ", GFX_GRAY_70);
    print("Init interrupts\n", GFX_ST_WHITE);
    // Clear IDT
    memset(&idt, 0, sizeof(idt));

    // Set ISR Handler (Exceptions 0-31)
    isr_install();

    // Set IRQ Handler (32-47)
    irq_install();

    // Load IDT
    idt_load();
}

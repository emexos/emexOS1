#include "irq.h"
#include <kernel/arch/x86_64/idt/idt.h>
#include <kernel/include/ports.h>

static irq_handler_t irq_handlers[16];
static volatile int irq_initialized = 0;

static void pic_remap(void)
{
    /*u8 mask1, mask2;

    // save masks
    //mask1 = inb(PIC1_DATA);
    //mask2 = inb(PIC2_DATA);
    */

    // ICW1
    outb(PIC1_COMMAND, 0x11);
    io_wait();
    outb(PIC2_COMMAND, 0x11);
    io_wait();

    // ICW2 Remap IRQs
    outb(PIC1_DATA, 0x20); // Master PIC vector offset (32)
    io_wait();
    outb(PIC2_DATA, 0x28); // Slave PIC vector offset (40)
    io_wait();

    // ICW3 cascading
    outb(PIC1_DATA, 0x04); // Master PIC > slave at IRQ2
    io_wait();
    outb(PIC2_DATA, 0x02); // Slave PIC its cascade identity
    io_wait();

    // ICW4
    outb(PIC1_DATA, 0x01); // 8086 mode
    io_wait();
    outb(PIC2_DATA, 0x01);
    io_wait();

    // Restore saved masks (disable all)
    outb(PIC1_DATA, 0xFF);
    io_wait();
    outb(PIC2_DATA, 0xFF);
    io_wait();
}

void irq_install(void)
{
    // Handler Array
    for (int i = 0; i < 16; i++) {
        irq_handlers[i] = NULL;
    }

    // Remap PIC
    pic_remap();

    // Set IDT Gates for all IRQs
    idt_set_gate(32, (u64)irq0, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(33, (u64)irq1, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(34, (u64)irq2, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(35, (u64)irq3, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(36, (u64)irq4, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(37, (u64)irq5, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(38, (u64)irq6, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(39, (u64)irq7, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(40, (u64)irq8, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(41, (u64)irq9, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(42, (u64)irq10, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(43, (u64)irq11, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(44, (u64)irq12, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(45, (u64)irq13, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(46, (u64)irq14, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(47, (u64)irq15, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);

    // Enable Timer (IRQ0) and Keyboard (IRQ1)
    irq_set_mask(0, 0); // Timer
    irq_set_mask(1, 0); // Keyboard

    irq_initialized = 1;
}

void irq_register_handler(u8 irq, irq_handler_t handler)
{
    if (irq < 16) {
        irq_handlers[irq] = handler;
        irq_set_mask(irq, 0); // enable
    }
}

void irq_unregister_handler(u8 irq)
{
    if (irq < 16) {
        irq_handlers[irq] = NULL;
        irq_set_mask(irq, 1); // disable
    }
}

void irq_set_mask(u8 irq, int enable)
{
    u16 port;
    u8 value;

    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }

    if (enable) {
        value = inb(port) | (1 << irq);
    } else {
        value = inb(port) & ~(1 << irq);
    }

    outb(port, value);
    io_wait();
}

void irq_ack(u8 irq)
{
    // Send EOI to PICs
    if (irq >= 8) {
        outb(PIC2_COMMAND, PIC_EOI);
    }
    outb(PIC1_COMMAND, PIC_EOI);
}

void irq_handler(cpu_state_t* state)
{
    if (!irq_initialized) {
        return;
    }

    u8 irq = state->int_no - 32;

    // when registered
    if (irq < 16 && irq_handlers[irq] != NULL) {
        irq_handlers[irq](state);
    }

    // Send EOI
    irq_ack(irq);
}

#ifndef IRQ_H
#define IRQ_H

#include "idt.h"
#include <types.h>

// IRQ Numbers
#define IRQ0  32    // Timer - used
#define IRQ1  33    // Keyboard - used
#define IRQ2  34    // Cascade - used
#define IRQ3  35    // COM2
#define IRQ4  36    // COM1
#define IRQ5  37    // LPT2
#define IRQ6  38    // Floppy
#define IRQ7  39    // LPT1
#define IRQ8  40    // RTC
#define IRQ9  41    // Free
#define IRQ10 42    // Free
#define IRQ11 43    // Free
#define IRQ12 44    // Mouse - un-...
#define IRQ13 45    // FPU
#define IRQ14 46    // Primary ATA
#define IRQ15 47    // Secondary ATA

// PIC Ports
#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

#define PIC_EOI      0x20

// IRQ Handler Type
typedef void (*irq_handler_t)(cpu_state_t* state);

void irq_install(void);
void irq_register_handler(u8 irq, irq_handler_t handler);
void irq_unregister_handler(u8 irq);
void irq_set_mask(u8 irq, int enable);
void irq_ack(u8 irq);

// IRQ Assembly Stubs irq.asm
extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);

#endif

#include "isr.h"
#include "exception_handler.h"
#include <kernel/cpu/idt.h>

static isr_handler_t isr_handlers[32];

void isr_install(void)
{
    // Handler Array
    for (int i = 0; i < 32; i++) {
        isr_handlers[i] = NULL;
    }

    // Set IDT Gates for ALL Exceptions
    idt_set_gate(0, (u64)isr0, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(1, (u64)isr1, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(2, (u64)isr2, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(3, (u64)isr3, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(4, (u64)isr4, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(5, (u64)isr5, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(6, (u64)isr6, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(7, (u64)isr7, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(8, (u64)isr8, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(9, (u64)isr9, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(10, (u64)isr10, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(11, (u64)isr11, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(12, (u64)isr12, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(13, (u64)isr13, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(14, (u64)isr14, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(15, (u64)isr15, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(16, (u64)isr16, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(17, (u64)isr17, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(18, (u64)isr18, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(19, (u64)isr19, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(20, (u64)isr20, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(21, (u64)isr21, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(22, (u64)isr22, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(23, (u64)isr23, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(24, (u64)isr24, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(25, (u64)isr25, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(26, (u64)isr26, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(27, (u64)isr27, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(28, (u64)isr28, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(29, (u64)isr29, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(30, (u64)isr30, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(31, (u64)isr31, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
}

void isr_register_handler(u8 num, isr_handler_t handler)
{
    if (num < 32) {
        isr_handlers[num] = handler;
    }
}

void isr_unregister_handler(u8 num)
{
    if (num < 32) {
        isr_handlers[num] = NULL;
    }
}

void isr_handler(cpu_state_t* state)
{
    if (state->int_no < 32 && isr_handlers[state->int_no] != NULL) {
        // Custom handler registered
        isr_handlers[state->int_no](state);
    } else {
        // No handler - panic
        exception_handler(state);
    }
}

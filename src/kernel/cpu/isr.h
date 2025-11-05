#ifndef ISR_H
#define ISR_H

#include "idt.h"
#include <types.h>

// Exception Messages
#define ISR_DIVISION_ERROR          0
#define ISR_DEBUG                   1
#define ISR_NMI                     2
#define ISR_BREAKPOINT              3
#define ISR_OVERFLOW                4
#define ISR_BOUND_RANGE             5
#define ISR_INVALID_OPCODE          6
#define ISR_DEVICE_NOT_AVAILABLE    7
#define ISR_DOUBLE_FAULT            8
#define ISR_COPROCESSOR_SEGMENT     9
#define ISR_INVALID_TSS             10
#define ISR_SEGMENT_NOT_PRESENT     11
#define ISR_STACK_FAULT             12
#define ISR_GENERAL_PROTECTION      13
#define ISR_PAGE_FAULT              14
#define ISR_RESERVED                15
#define ISR_FPU_ERROR               16
#define ISR_ALIGNMENT_CHECK         17
#define ISR_MACHINE_CHECK           18
#define ISR_SIMD_FP_EXCEPTION       19
#define ISR_VIRTUALIZATION          20
#define ISR_SECURITY_EXCEPTION      30

// ISR Handler Type
typedef void (*isr_handler_t)(cpu_state_t* state);

void isr_install(void);
void isr_register_handler(u8 num, isr_handler_t handler);
void isr_unregister_handler(u8 num);

// ISR Assembly Stubs
extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);
extern void isr23(void);
extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);

#endif

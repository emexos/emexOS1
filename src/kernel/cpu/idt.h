#ifndef IDT_H
#define IDT_H

#include "../../../shared/types.h"

#define IDT_ENTRIES 256

// IDT Entry Flags
#define IDT_FLAG_PRESENT    0x80
#define IDT_FLAG_RING0      0x00
#define IDT_FLAG_RING3      0x60
#define IDT_FLAG_GATE_INT   0x0E
#define IDT_FLAG_GATE_TRAP  0x0F

// IDT Entry Structure
typedef struct {
    u16 offset_low;
    u16 selector;
    u8 ist;
    u8 flags;
    u16 offset_mid;
    u32 offset_high;
    u32 reserved;
} __attribute__((packed)) idt_entry_t;

// IDT Pointer Structure
typedef struct {
    u16 limit;
    u64 base;
} __attribute__((packed)) idt_ptr_t;

// Interrupt Frame
typedef struct {
    u64 rip;
    u64 cs;
    u64 rflags;
    u64 rsp;
    u64 ss;
} __attribute__((packed)) interrupt_frame_t;

// CPU State für Context Switching
typedef struct {
    u64 r15, r14, r13, r12, r11, r10, r9, r8;
    u64 rbp, rdi, rsi, rdx, rcx, rbx, rax;
    u64 int_no, err_code;
    u64 rip, cs, rflags, rsp, ss;
} __attribute__((packed)) cpu_state_t;

// Funktionen
void idt_init(void);
void idt_set_gate(u8 num, u64 handler, u8 flags);
void idt_load(void);

#endif

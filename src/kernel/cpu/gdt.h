#ifndef GDT_H
#define GDT_H

#include "../../../shared/types.h"

#define GDT_ENTRIES 7

// GDT Entry Flags
#define GDT_PRESENT     0x80
#define GDT_RING0       0x00
#define GDT_RING3       0x60
#define GDT_SYSTEM      0x00
#define GDT_CODE_DATA   0x10
#define GDT_EXECUTABLE  0x08
#define GDT_RW          0x02
#define GDT_ACCESSED    0x01

// GDT Granularity Flags
#define GDT_GRANULAR    0x80  // 4KB blocks
#define GDT_LONG_MODE   0x20  // 64-bit
#define GDT_32BIT       0x40  // 32-bit protected mode

// Segment Selectors
#define KERNEL_CODE_SELECTOR  0x08
#define KERNEL_DATA_SELECTOR  0x10
#define USER_CODE_SELECTOR    0x18
#define USER_DATA_SELECTOR    0x20
#define TSS_SELECTOR          0x28

// GDT Entry Structure (64-bit)
typedef struct {
    u16 limit_low;
    u16 base_low;
    u8  base_mid;
    u8  access;
    u8  granularity;
    u8  base_high;
} __attribute__((packed)) gdt_entry_t;

// TSS Entry Structure (64-bit)
typedef struct {
    u16 limit_low;
    u16 base_low;
    u8  base_mid;
    u8  access;
    u8  granularity;
    u8  base_high;
    u32 base_upper;
    u32 reserved;
} __attribute__((packed)) gdt_tss_entry_t;

// GDT Pointer Structure
typedef struct {
    u16 limit;
    u64 base;
} __attribute__((packed)) gdt_ptr_t;

// Task State Segment (64-bit)
typedef struct {
    u32 reserved0;
    u64 rsp0;  // Kernel stack pointer
    u64 rsp1;
    u64 rsp2;
    u64 reserved1;
    u64 ist1;
    u64 ist2;
    u64 ist3;
    u64 ist4;
    u64 ist5;
    u64 ist6;
    u64 ist7;
    u64 reserved2;
    u16 reserved3;
    u16 iopb_offset;
} __attribute__((packed)) tss_t;

// Functions
void gdt_init(void);
void gdt_set_kernel_stack(u64 stack);
void gdt_load(void);
void tss_init(void);
void tss_set_stack(u64 stack);

#endif

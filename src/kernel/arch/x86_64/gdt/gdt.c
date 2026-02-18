#include "gdt.h"
#include <memory/main.h>
#include <string/string.h>
#include <theme/stdclrs.h>
#include <kernel/graph/theme.h>
#include <theme/doccr.h>

static gdt_entry_t gdt[GDT_ENTRIES];
static tss_t tss;
static gdt_ptr_t gdt_ptr;
static u8 kernel_stack[16384] __attribute__((aligned(16))); // 16KB
static u8 df_stack[8192] __attribute__((aligned(16))); // this time via ist1

// TODO:
// move tss to kernel/arch/x64/tss/tss.c


extern void gdt_flush(u64);
extern void tss_flush(u16);

static void gdt_set_gate(int num, u32 base, u32 limit, u8 access, u8 gran)
{
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_mid = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = (limit >> 16) & 0x0F;
    gdt[num].granularity |= gran & 0xF0;
    gdt[num].access = access;
}

static void tss_set_entry(void)
{
    u64 base = (u64)&tss;
    u32 limit = sizeof(tss) - 1;

    // Cast GDT entry to TSS entry for first part
    gdt_tss_entry_t* tss_gdt = (gdt_tss_entry_t*)&gdt[5];

    tss_gdt->limit_low = limit & 0xFFFF;
    tss_gdt->base_low = base & 0xFFFF;
    tss_gdt->base_mid = (base >> 16) & 0xFF;
    tss_gdt->access = 0x89; // Present, Ring 0, TSS
    tss_gdt->granularity = 0x00;
    tss_gdt->base_high = (base >> 24) & 0xFF;
    tss_gdt->base_upper = (base >> 32) & 0xFFFFFFFF;
    tss_gdt->reserved = 0;
}

void gdt_init(void)
{
    log("[GDT]", "init (Global Descriptor Table)\n", d);
    gdt_ptr.limit = sizeof(gdt) - 1;
    gdt_ptr.base = (u64)&gdt;

    // NULL descriptor (0x00)
    gdt_set_gate(0, 0, 0, 0, 0);

    // kernel code Segment (0x08)
    gdt_set_gate(1, 0, 0xFFFFF,
                 GDT_PRESENT | GDT_RING0 | GDT_CODE_DATA | GDT_EXECUTABLE | GDT_RW,
                 GDT_GRANULAR | GDT_LONG_MODE);

    // kernel data Segment (0x10)
    gdt_set_gate(2, 0, 0xFFFFF,
                 GDT_PRESENT | GDT_RING0 | GDT_CODE_DATA | GDT_RW,
                 GDT_GRANULAR | GDT_LONG_MODE);

    // user data Segment (0x18)
    gdt_set_gate(3, 0, 0xFFFFF,
                 GDT_PRESENT | GDT_RING3 | GDT_CODE_DATA | GDT_RW,
                 GDT_GRANULAR | GDT_LONG_MODE);

    // user code Segment (0x20)
    gdt_set_gate(4, 0, 0xFFFFF,
                 GDT_PRESENT | GDT_RING3 | GDT_CODE_DATA | GDT_EXECUTABLE | GDT_RW,
                 GDT_GRANULAR | GDT_LONG_MODE);

    // TSS Segment (0x28) 2 entrys
    tss_init();
    tss_set_entry();

    gdt_flush((u64)&gdt_ptr);
    tss_flush(TSS_SELECTOR);
}

void tss_init(void)
{
    log("[TSS]", "init (Task State Segment)\n", d);
    memset(&tss, 0, sizeof(tss_t));
    tss.rsp0 = (u64)kernel_stack + sizeof(kernel_stack);
    tss.ist1 = (u64)df_stack + sizeof(df_stack);
    tss.iopb_offset = sizeof(tss_t);
}

void tss_set_stack(u64 stack){ tss.rsp0 = stack;}
void gdt_set_kernel_stack(u64 stack){tss_set_stack(stack);}

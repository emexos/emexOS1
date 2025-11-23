#include "gdt.h"
#include <klib/memory/main.h>
#include <klib/string/string.h>
#include <theme/stdclrs.h>
#include <klib/graphics/theme.h>
#include <theme/doccr.h>

static gdt_entry_t gdt[GDT_ENTRIES];
//static gdt_tss_entry_t tss_entry;
static tss_t tss;
static gdt_ptr_t gdt_ptr;

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
    BOOTUP_PRINT("[GDT] ", GFX_GRAY_70);
    BOOTUP_PRINT("init (Global Descriptor Table)\n", GFX_ST_WHITE);
    // Setup GDT pointer
    gdt_ptr.limit = sizeof(gdt) - 1;
    gdt_ptr.base = (u64)&gdt;

    // NULL descriptor (0x00)
    gdt_set_gate(0, 0, 0, 0, 0);

    // Kernel Code Segment (0x08)
    gdt_set_gate(1, 0, 0xFFFFF,
                 GDT_PRESENT | GDT_RING0 | GDT_CODE_DATA | GDT_EXECUTABLE | GDT_RW,
                 GDT_GRANULAR | GDT_LONG_MODE);

    // Kernel Data Segment (0x10)
    gdt_set_gate(2, 0, 0xFFFFF,
                 GDT_PRESENT | GDT_RING0 | GDT_CODE_DATA | GDT_RW,
                 GDT_GRANULAR | GDT_LONG_MODE);

    // User Code Segment (0x18)
    gdt_set_gate(3, 0, 0xFFFFF,
                 GDT_PRESENT | GDT_RING3 | GDT_CODE_DATA | GDT_EXECUTABLE | GDT_RW,
                 GDT_GRANULAR | GDT_LONG_MODE);

    // User Data Segment (0x20)
    gdt_set_gate(4, 0, 0xFFFFF,
                 GDT_PRESENT | GDT_RING3 | GDT_CODE_DATA | GDT_RW,
                 GDT_GRANULAR | GDT_LONG_MODE);

    // TSS Segment (0x28) - takes 2 entries in 64-bit mode
    tss_init();
    tss_set_entry();

    // Load GDT
    gdt_flush((u64)&gdt_ptr);

    // Load TSS
    tss_flush(TSS_SELECTOR);
}

void tss_init(void)
{
    memset(&tss, 0, sizeof(tss_t));

    // Set IOPB to size of TSS (no I/O permission bitmap)
    tss.iopb_offset = sizeof(tss_t);
}

void tss_set_stack(u64 stack)
{
    tss.rsp0 = stack;
}

void gdt_set_kernel_stack(u64 stack)
{
    tss_set_stack(stack);
}

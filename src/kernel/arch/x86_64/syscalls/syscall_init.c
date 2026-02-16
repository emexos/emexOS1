#include "syscall_init.h"

#include <kernel/arch/x86_64/gdt/gdt.h>
#include <kernel/communication/serial.h>
#include <kernel/graph/theme.h>
#include <theme/doccr.h>
#include <string/string.h>
#include <types.h>

// MSR addresses
#define MSR_EFER 0xC0000080
#define MSR_STAR 0xC0000081
#define MSR_LSTAR 0xC0000082
#define MSR_SFMASK 0xC0000084

// write MSR
static inline void wrmsr(u32 msr, u64 value) {
    u32 low = value & 0xFFFFFFFF;
    u32 high = value >> 32;
    __asm__ volatile("wrmsr" : : "c"(msr), "a"(low), "d"(high));
}

// read MSR
static inline u64 rdmsr(u32 msr) {
    u32 low, high;
    __asm__ volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return ((u64)high << 32) | low;
}

void syscall_arch_init(void) {
    log("[SYSCALL]", "initializing SYSCALL/SYSRET\n", d);

    // enable SYSCALL in EFER
    u64 efer = rdmsr(MSR_EFER);
    efer |= (1 << 0);  // SCE bit
    wrmsr(MSR_EFER, efer);

    // bits 63:48 == kernel CS/SS for SYSRET
    // bits 47:32 == user CS/SS for SYSCALL
    u64 star = 0;
    star |= ((u64)0x08 << 32);
    star |= ((u64)0x10 << 48);

    wrmsr(MSR_STAR, star);
    wrmsr(MSR_LSTAR, (u64)syscall_entry);
    wrmsr(MSR_SFMASK, 0x200);

    log("[SYSCALL]", "STAR=0x", d);
    char buf[32];
    buf[0] = '\0';
    str_from_hex(buf, star);
    BOOTUP_PRINT(buf, white());
    BOOTUP_PRINT("\n", white());

    log("[SYSCALL]", "enabled\n", success);
}

#include "cpu.h"
#include <string/string.h>
#include <memory/main.h>
#include <theme/stdclrs.h>
#include <kernel/graph/theme.h>
#include <theme/doccr.h>

static cpu_info_t cpu_info;

//this only works on qemu and may some other emulators

static void cpuid(u32 code, u32 *a, u32 *b, u32 *c, u32 *d) {
    __asm__ volatile("cpuid"
        : "=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d)
        //: "a"(code));
        : "a"(code), "c"(0)) ;
}

static void cpuid_ext(u32 code, u32 subcode, u32 *a, u32 *b, u32 *c, u32 *d) {
    __asm__ volatile("cpuid"
        : "=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d)
        : "a"(code), "c"(subcode));
}

void cpu_detect(void) {
    BOOTUP_PRINT("[CPU] ", GFX_GRAY_70);

    memset(&cpu_info, 0, sizeof(cpu_info_t));
    u32 eax, ebx, ecx, edx;

    // Get vendor string
    cpuid(0, &eax, &ebx, &ecx, &edx);
    *(u32*)(cpu_info.vendor + 0) = ebx;
    *(u32*)(cpu_info.vendor + 4) = edx;
    *(u32*)(cpu_info.vendor + 8) = ecx;
    cpu_info.vendor[12] = '\0';

    // Get features
    cpuid(1, &eax, &ebx, &ecx, &edx);
    cpu_info.features_edx = edx;
    cpu_info.features_ecx = ecx;

    cpu_info.stepping = eax & 0xF;

    cpu_info.model = (eax >> 4) & 0xF;
    cpu_info.family = (eax >> 8) & 0xF;
    if (cpu_info.family == 0xF) {
        cpu_info.family += (eax >> 20) & 0xFF;
    }
    if (cpu_info.family == 0x6 || cpu_info.family == 0xF) {
        cpu_info.model += ((eax >> 16) & 0xF) << 4;
    }

    // cache & threads
    cpu_info.cache_line_size = ((ebx >> 8) & 0xFF) * 8;
    cpu_info.threads = (ebx >> 16) & 0xFF;

    cpuid(7, &eax, &ebx, &ecx, &edx);
    cpu_info.extended_features_ebx = ebx;
    cpu_info.extended_features_ecx = ecx;

    // Get brand string if available
    cpuid(0x80000000, &eax, &ebx, &ecx, &edx);
    if (eax >= 0x80000004) {
        u32 *brand_ptr = (u32*)cpu_info.brand;

        cpuid(0x80000002, &eax, &ebx, &ecx, &edx);
        brand_ptr[0] = eax; brand_ptr[1] = ebx;
        brand_ptr[2] = ecx; brand_ptr[3] = edx;

        cpuid(0x80000003, &eax, &ebx, &ecx, &edx);
        brand_ptr[4] = eax; brand_ptr[5] = ebx;
        brand_ptr[6] = ecx; brand_ptr[7] = edx;

        cpuid(0x80000004, &eax, &ebx, &ecx, &edx);
        brand_ptr[8] = eax; brand_ptr[9] = ebx;
        brand_ptr[10] = ecx; brand_ptr[11] = edx;

        cpu_info.brand[48] = '\0';

        // Trim leading spaces
        char *start = cpu_info.brand;
        while (*start == ' ') start++;
        if (start != cpu_info.brand) {
            int i = 0;
            while (start[i]) {
                cpu_info.brand[i] = start[i];
                i++;
            }
            cpu_info.brand[i] = '\0';
        }
    }

    // cache info for Intel
    if (cpu_info.vendor[0] == 'G')
    { // (GenuineIntel) x86_84
        cpuid(4, &eax, &ebx, &ecx, &edx);

        // L1 data cache
        cpuid_ext(4, 0, &eax, &ebx, &ecx, &edx);
        if ((eax & 0x1F) == 1) { // data cache
            u32 ways = ((ebx >> 22) & 0x3FF) + 1;
            u32 partitions = ((ebx >> 12) & 0x3FF) + 1;
            u32 line_size = (ebx & 0xFFF) + 1;
            u32 sets = ecx + 1;
            cpu_info.cache_l1d = (ways * partitions * line_size * sets) / 1024;
        }

        // L1 instruction cache
        cpuid_ext(4, 1, &eax, &ebx, &ecx, &edx);
        if ((eax & 0x1F) == 2) { // instruction cache
            u32 ways = ((ebx >> 22) & 0x3FF) + 1;
            u32 partitions = ((ebx >> 12) & 0x3FF) + 1;
            u32 line_size = (ebx & 0xFFF) + 1;
            u32 sets = ecx + 1;
            cpu_info.cache_l1i = (ways * partitions * line_size * sets) / 1024;
        }

        // L2 cache
        cpuid_ext(4, 2, &eax, &ebx, &ecx, &edx);
        if ((eax & 0x1F) == 3) { // unified cache
            u32 ways = ((ebx >> 22) & 0x3FF) + 1;
            u32 partitions = ((ebx >> 12) & 0x3FF) + 1;
            u32 line_size = (ebx & 0xFFF) + 1;
            u32 sets = ecx + 1;
            cpu_info.cache_l2 = (ways * partitions * line_size * sets) / 1024;
        }

        // L3 cache
        cpuid_ext(4, 3, &eax, &ebx, &ecx, &edx);
        if ((eax & 0x1F) == 3) {
            u32 ways = ((ebx >> 22) & 0x3FF) + 1;
            u32 partitions = ((ebx >> 12) & 0x3FF) + 1;
            u32 line_size = (ebx & 0xFFF) + 1;
            u32 sets = ecx + 1;
            cpu_info.cache_l3 = (ways * partitions * line_size * sets) / 1024;
        }
    }
    else if (cpu_info.vendor[0] == 'A')
    { // (AuthenticAMD) == for macbooks or others
        cpuid(0x80000005, &eax, &ebx, &ecx, &edx);
        cpu_info.cache_l1d = (ecx >> 24) & 0xFF;
        cpu_info.cache_l1i = (edx >> 24) & 0xFF;

        cpuid(0x80000006, &eax, &ebx, &ecx, &edx);
        cpu_info.cache_l2 = (ecx >> 16) & 0xFFFF;//   KB
        cpu_info.cache_l3 = ((edx >> 18) & 0x3FFF) * 512;
    }

    // core count
    cpuid(0x80000008, &eax, &ebx, &ecx, &edx);
    cpu_info.cores = (ecx & 0xFF) + 1;

    BOOTUP_PRINT("detected: ", white());
    BOOTUP_PRINT(cpu_info.brand, white());
    BOOTUP_PRINT("\n", white());

    BOOTUP_PRINT("  Vendor: ", white());
    BOOTUP_PRINT(cpu_info.vendor, white());
    BOOTUP_PRINT("\n", white());

    BOOTUP_PRINT("  Cores: ", white());
    BOOTUP_PRINT_INT(cpu_info.cores, white());
    BOOTUP_PRINT("\n", white());

    BOOTUP_PRINT("  Threads: ", white());
    BOOTUP_PRINT_INT(cpu_info.threads, white());
    BOOTUP_PRINT("\n", white());
}

const char* cpu_get_vendor(void) {
    return cpu_info.vendor;
}

const char* cpu_get_brand(void) {
    return cpu_info.brand[0] ? cpu_info.brand : cpu_info.vendor;
}

cpu_info_t* cpu_get_info(void) {
    return &cpu_info;
}

int cpu_has_feature(u32 feature) {
    // check on EDX features
    if (feature & 0xFFFF0000) {
        return (cpu_info.features_edx & feature) != 0;
    } return (cpu_info.features_ecx & feature) != 0;
}

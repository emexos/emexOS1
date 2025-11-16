#include "cpu.h"

static char cpu_vendor[13] = {0};
static char cpu_brand[49] = {0};

//this only works on qemu and may some other emulators

static void cpuid(u32 code, u32 *a, u32 *b, u32 *c, u32 *d) {
    __asm__ volatile("cpuid"
        : "=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d)
        : "a"(code));
}

void cpu_detect(void) {
    u32 eax, ebx, ecx, edx;

    // Get vendor string
    cpuid(0, &eax, &ebx, &ecx, &edx);

    *(u32*)(cpu_vendor + 0) = ebx;
    *(u32*)(cpu_vendor + 4) = edx;
    *(u32*)(cpu_vendor + 8) = ecx;
    cpu_vendor[12] = '\0';

    // Get brand string if available
    cpuid(0x80000000, &eax, &ebx, &ecx, &edx);
    if (eax >= 0x80000004) {
        u32 *brand_ptr = (u32*)cpu_brand;

        cpuid(0x80000002, &eax, &ebx, &ecx, &edx);
        brand_ptr[0] = eax; brand_ptr[1] = ebx;
        brand_ptr[2] = ecx; brand_ptr[3] = edx;

        cpuid(0x80000003, &eax, &ebx, &ecx, &edx);
        brand_ptr[4] = eax; brand_ptr[5] = ebx;
        brand_ptr[6] = ecx; brand_ptr[7] = edx;

        cpuid(0x80000004, &eax, &ebx, &ecx, &edx);
        brand_ptr[8] = eax; brand_ptr[9] = ebx;
        brand_ptr[10] = ecx; brand_ptr[11] = edx;

        cpu_brand[48] = '\0';

        // Trim leading spaces
        char *start = cpu_brand;
        while (*start == ' ') start++;
        if (start != cpu_brand) {
            int i = 0;
            while (start[i]) {
                cpu_brand[i] = start[i];
                i++;
            }
            cpu_brand[i] = '\0';
        }
    } else {
        cpu_brand[0] = '\0';
    }
}

const char* cpu_get_vendor(void) {
    return cpu_vendor;
}

const char* cpu_get_brand(void) {
    return cpu_brand[0] ? cpu_brand : cpu_vendor;
}

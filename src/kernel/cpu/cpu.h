#ifndef CPU_H
#define CPU_H

#include <types.h>

// CPUFF
#define CPU_FEATURE_FPU     (1 << 0)
#define CPU_FEATURE_VME     (1 << 1)
#define CPU_FEATURE_DE      (1 << 2)
#define CPU_FEATURE_PSE     (1 << 3)
#define CPU_FEATURE_TSC     (1 << 4)
#define CPU_FEATURE_MSR     (1 << 5)
#define CPU_FEATURE_PAE     (1 << 6)
#define CPU_FEATURE_MCE     (1 << 7)
#define CPU_FEATURE_CX8     (1 << 8)
#define CPU_FEATURE_APIC    (1 << 9)
#define CPU_FEATURE_SEP     (1 << 11)
#define CPU_FEATURE_MTRR    (1 << 12)
#define CPU_FEATURE_PGE     (1 << 13)
#define CPU_FEATURE_MCA     (1 << 14)
#define CPU_FEATURE_CMOV    (1 << 15)
#define CPU_FEATURE_PAT     (1 << 16)
#define CPU_FEATURE_PSE36   (1 << 17)
#define CPU_FEATURE_PSN     (1 << 18)
#define CPU_FEATURE_CLFSH   (1 << 19)
#define CPU_FEATURE_MMX     (1 << 23)
#define CPU_FEATURE_FXSR    (1 << 24)
#define CPU_FEATURE_SSE     (1 << 25)
#define CPU_FEATURE_SSE2    (1 << 26)
#define CPU_FEATURE_HTT     (1 << 28)

// ECX lf 0
#define CPU_FEATURE_SSE3    (1 << 0)
#define CPU_FEATURE_PCLMUL  (1 << 1)
#define CPU_FEATURE_SSSE3   (1 << 9)
#define CPU_FEATURE_FMA     (1 << 12)
#define CPU_FEATURE_CX16    (1 << 13)
#define CPU_FEATURE_SSE41   (1 << 19)
#define CPU_FEATURE_SSE42   (1 << 20)
#define CPU_FEATURE_POPCNT  (1 << 23)
#define CPU_FEATURE_AES     (1 << 25)
#define CPU_FEATURE_XSAVE   (1 << 26)
#define CPU_FEATURE_AVX     (1 << 28)
#define CPU_FEATURE_F16C    (1 << 29)
#define CPU_FEATURE_RDRAND  (1 << 30)

// EBX for leaf 7
#define CPU_FEATURE_AVX2    (1 << 5)
#define CPU_FEATURE_BMI1    (1 << 3)
#define CPU_FEATURE_BMI2    (1 << 8)
#define CPU_FEATURE_AVX512F (1 << 16)

typedef struct {
    char vendor[13];
    char brand[49];
    u32 features_edx;
    u32 features_ecx;
    u32 extended_features_ebx;
    u32 extended_features_ecx;
    u32 family;
    u32 model;
    u32 stepping;
    u32 cache_line_size;
    u32 cores;
    u32 threads;
    u32 cache_l1d;
    u32 cache_l1i;
    u32 cache_l2;
    u32 cache_l3;

    //amd save boot
    u8 has_long_mode;
    u8 has_nx;
} cpu_info_t;

void cpu_detect(void);
const char* cpu_get_vendor(void);
const char* cpu_get_brand(void);
cpu_info_t* cpu_get_info(void);
int cpu_has_feature(u32 feature);

void cli(void);
void sti(void);
__attribute__((noreturn)) void halt(void);
__attribute__((noreturn)) void idle(void);
void wfi(void);
void nop(void);

#endif

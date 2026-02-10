#ifndef AMD64_H
#define AMD64_H

#include <types.h>

// leafes
#define AMD_LEAF_EXTENDED_FEATURES 0x80000001
#define AMD_LEAF_BRAND_STRING_1    0x80000002
#define AMD_LEAF_BRAND_STRING_2    0x80000003
#define AMD_LEAF_BRAND_STRING_3    0x80000004
#define AMD_LEAF_L1_CACHE          0x80000005
#define AMD_LEAF_L2_L3_CACHE       0x80000006
#define AMD_LEAF_APM               0x80000007
#define AMD_LEAF_ADDRESS_SIZES     0x80000008

// EDX
#define AMD_FEATURE_SYSCALL    (1 << 11) // SYSCALL/SYSRET
#define AMD_FEATURE_NX         (1 << 20)
#define AMD_FEATURE_MMXEXT     (1 << 22)
#define AMD_FEATURE_FXSR       (1 << 24) // normal
#define AMD_FEATURE_FFXSR      (1 << 25) // fast
#define AMD_FEATURE_1GB_PAGE   (1 << 26)
#define AMD_FEATURE_RDTSCP     (1 << 27)
#define AMD_FEATURE_LM         (1 << 29) // long mode
#define AMD_FEATURE_3DNOWEXT   (1 << 30) // extensions
#define AMD_FEATURE_3DNOW      (1U << 31)

// ECX
#define AMD_FEATURE_LAHF_LM    (1 << 0)
#define AMD_FEATURE_CMP_LEGACY (1 << 1)
#define AMD_FEATURE_SVM        (1 << 2)
#define AMD_FEATURE_EXTAPIC    (1 << 3)
#define AMD_FEATURE_CR8_LEGACY (1 << 4)
#define AMD_FEATURE_ABM        (1 << 5) // (LZCNT)
#define AMD_FEATURE_SSE4A      (1 << 6)
#define AMD_FEATURE_MISALIGNSSE (1 << 7)
#define AMD_FEATURE_3DNOWPREFETCH (1 << 8) // PREFETCH/PREFETCHW
#define AMD_FEATURE_OSVW       (1 << 9) // OS Visible Workaround
#define AMD_FEATURE_IBS        (1 << 10)
#define AMD_FEATURE_XOP        (1 << 11) // Extended Operations
#define AMD_FEATURE_SKINIT     (1 << 12)
#define AMD_FEATURE_WDT        (1 << 13)
#define AMD_FEATURE_LWP        (1 << 15)
#define AMD_FEATURE_FMA4       (1 << 16)
#define AMD_FEATURE_TBM        (1 << 21)
#define AMD_FEATURE_TOPOEXT    (1 << 22)

// EDX: leaf 0x80000007
#define AMD_APM_TS             (1 << 0)
#define AMD_APM_FID            (1 << 1)
#define AMD_APM_VID            (1 << 2)
#define AMD_APM_TTP            (1 << 3) // THERMTRIP
#define AMD_APM_TM             (1 << 4) // Hardware Thermal Control
#define AMD_APM_STC            (1 << 5) // Software Thermal Control
#define AMD_APM_100MHZSTEPS    (1 << 6)
#define AMD_APM_HWPSTATE       (1 << 7) // P-State
#define AMD_APM_TSC_INVARIANT  (1 << 8)

typedef struct {
    u32 ext_features_edx;
    u32 ext_features_ecx;
    u32 apm_features; // power mgt

    u32 l1_data_cache_kb;
    u32 l1_inst_cache_kb;
    u32 l2_cache_kb;
    u32 l3_cache_kb;
    u8 phys_addr_bits;
    u8 virt_addr_bits;
    u8 guest_phys_addr_bits;

    u8 family;
    u8 model;
    u8 stepping;

    u8 has_svm;
    u8 has_sse4a;
    u8 has_xop;
    u8 has_fma4;
    u8 has_tbm;
    u8 has_1gb_pages;
    u8 has_invariant_tsc;
} amd64_info_t;

void amd64_detect(amd64_info_t *info);
void amd64_print_info(const amd64_info_t *info);

// random stuf:
void amd64_init_optimizations(void);
int amd64_has_feature(const amd64_info_t *info, u32 feature);
int amd64_has_apm_feature(const amd64_info_t *info, u32 feature);

#endif

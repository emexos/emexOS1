#include "amd64.h"
#include <memory/main.h>
#include <string/string.h>
#include <kernel/communication/serial.h>
#include <theme/stdclrs.h>

static void cpuid(u32 leaf, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx) {
    __asm__ volatile("cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(leaf), "c"(0));
}

static void cpuid_count(u32 leaf, u32 subleaf, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx) {
    __asm__ volatile("cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(leaf), "c"(subleaf));
}

void amd64_detect(amd64_info_t *info) {
    if (!info) return;

    memset(info, 0, sizeof(amd64_info_t));

    u32 eax, ebx, ecx, edx;

    // extended CPUID
    cpuid(0x80000000, &eax, &ebx, &ecx, &edx);
    u32 max_ext_leaf = eax;

    if (max_ext_leaf < AMD_LEAF_EXTENDED_FEATURES) {
        log("[AMD64]", "extended CPUID not available!\n", d);
        return;
    }

    cpuid(AMD_LEAF_EXTENDED_FEATURES, &eax, &ebx, &ecx, &edx);
    info->ext_features_edx = edx;
    info->ext_features_ecx = ecx;

    // extract family, model , stepping
    cpuid(1, &eax, &ebx, &ecx, &edx);
    info->stepping = eax & 0xF;
    info->model = (eax >> 4) & 0xF;
    info->family = (eax >> 8) & 0xF;

    // family/model but for AMD
    if (info->family == 0xF) {
        info->family += (eax >> 20) & 0xFF;
    }
    if (info->family == 0xF || info->family >= 0x10) {
        info->model += ((eax >> 16) & 0xF) << 4;
    }

    // "L1 cache" info
    if (max_ext_leaf >= AMD_LEAF_L1_CACHE) {
        cpuid(AMD_LEAF_L1_CACHE, &eax, &ebx, &ecx, &edx);
        info->l1_data_cache_kb = (ecx >> 24) & 0xFF;
        info->l1_inst_cache_kb = (edx >> 24) & 0xFF;
    }

    // "L2/L3 cache" info
    if (max_ext_leaf >= AMD_LEAF_L2_L3_CACHE) {
        cpuid(AMD_LEAF_L2_L3_CACHE, &eax, &ebx, &ecx, &edx);
        info->l2_cache_kb = (ecx >> 16) & 0xFFFF;
        info->l3_cache_kb = ((edx >> 18) & 0x3FFF) * 512; // In 512KB units
    }
    if (max_ext_leaf >= AMD_LEAF_APM) { // {APM} << Advanced Power Management
        cpuid(AMD_LEAF_APM, &eax, &ebx, &ecx, &edx);
        info->apm_features = edx;
    }

    // address sizes
    if (max_ext_leaf >= AMD_LEAF_ADDRESS_SIZES) {
        cpuid(AMD_LEAF_ADDRESS_SIZES, &eax, &ebx, &ecx, &edx);
        info->phys_addr_bits = eax & 0xFF;
        info->virt_addr_bits = (eax >> 8) & 0xFF;
        info->guest_phys_addr_bits = (eax >> 16) & 0xFF;
    }

    // setf eature flags
    info->has_svm = (info->ext_features_ecx & AMD_FEATURE_SVM) != 0;
    info->has_sse4a = (info->ext_features_ecx & AMD_FEATURE_SSE4A) != 0;
    info->has_xop = (info->ext_features_ecx & AMD_FEATURE_XOP) != 0;
    info->has_fma4 = (info->ext_features_ecx & AMD_FEATURE_FMA4) != 0;
    info->has_tbm = (info->ext_features_ecx & AMD_FEATURE_TBM) != 0;
    info->has_1gb_pages = (info->ext_features_edx & AMD_FEATURE_1GB_PAGE) != 0;
    info->has_invariant_tsc = (info->apm_features & AMD_APM_TSC_INVARIANT) != 0;
}

void amd64_print_info(const amd64_info_t *info) {
    if (!info) return;

    log("[AMD64]", "CPU info:\n", d);

    print("        Family: ", white());
    printInt(info->family, white());
    print(", Model: ", white());
    printInt(info->model, white());
    print(", Stepping: ", white());
    printInt(info->stepping, white());
    print("\n", white());

    print("        Address Sizes: ", white());
    printInt(info->phys_addr_bits, white());
    print("-bit physical, ", white());
    printInt(info->virt_addr_bits, white());
    print("-bit virtual\n", white());

    if (info->guest_phys_addr_bits > 0) {
        print("        Guest Physical: ", white());
        printInt(info->guest_phys_addr_bits, white());
        print("-bit\n", white());
    }

    log("\n", "       Cache Sizes:\n", d);

    print("          L1 Data:  ", white());
    printInt(info->l1_data_cache_kb, white());
    print(" KB\n", white());

    print("          L1 Inst:  ", white());
    printInt(info->l1_inst_cache_kb, white());
    print(" KB\n", white());

    print("          L2:       ", white());
    printInt(info->l2_cache_kb, white());
    print(" KB\n", white());

    if (info->l3_cache_kb > 0) {
        print("          L3:       ", white());
        printInt(info->l3_cache_kb, white());
        print(" KB\n", white());
    }

    log("\n", "       AMD-Specific Features:\n", d);

    if (info->has_svm)            log("", "          - AMD-V (SVM) virtualization\n", d);
    if (info->has_sse4a)          log("", "          - SSE4a\n", d);
    if (info->has_xop)            log("", "          - XOP instructions\n", d);
    if (info->has_fma4)           log("", "          - FMA4 instructions\n", d);
    if (info->has_tbm)            log("", "          - TBM instructions\n", d);
    if (info->has_1gb_pages)      log("", "          - 1GB page support\n", d);
    if (info->has_invariant_tsc)  log("", "          - Invariant TSC\n", d);

    if (info->ext_features_edx & AMD_FEATURE_NX)
        log("", "          - NX (No-Execute) bit\n", d);

    if (info->ext_features_edx & AMD_FEATURE_RDTSCP)
        log("", "          - RDTSCP instruction\n", d);

    if (info->ext_features_ecx & AMD_FEATURE_ABM)
        log("", "          - Advanced Bit Manipulation (LZCNT)\n", d);
}

int amd64_has_feature(const amd64_info_t *info, u32 feature) {
    if (!info) return 0;

    // check if feature is in EDX or ECX
    if (feature & 0x80000000) {
        // high bit set should mean its an EDX feature
        return (info->ext_features_edx & feature) != 0;
    } else {
        return (info->ext_features_ecx & feature) != 0;
    }
}

int amd64_has_apm_feature(const amd64_info_t *info, u32 feature) {
    if (!info) return 0;
    return (info->apm_features & feature) != 0;
}

void amd64_init_optimizations(void) {
    u32 eax, ebx, ecx, edx;

    // check for AMD CPU
    cpuid(0, &eax, &ebx, &ecx, &edx);
    if (!(ebx == 0x68747541 && edx == 0x69746E65 && ecx == 0x444D4163)) {
        // Not "AuthenticAMD"
        return;
    }

    log("[AMD64]", "AMD specific optimizations\n", d);

    // extended features
    cpuid(0x80000000, &eax, &ebx, &ecx, &edx);
    if (eax < AMD_LEAF_EXTENDED_FEATURES) {
        return;
    }

    cpuid(AMD_LEAF_EXTENDED_FEATURES, &eax, &ebx, &ecx, &edx);

    // enable NX bit if available
    if (edx & AMD_FEATURE_NX) {
        // NX bit is enabled via IA32_EFER MSR (bit 11)
        u32 efer_low, efer_high;
        __asm__ volatile("rdmsr" : "=a"(efer_low), "=d"(efer_high) : "c"(0xC0000080));
        efer_low |= (1 << 11); // set NXE bit
        __asm__ volatile("wrmsr" : : "a"(efer_low), "d"(efer_high), "c"(0xC0000080));
        printf("[AMD64] NX bit enabled\n");
    }


    // saftey checks
    if (edx & AMD_FEATURE_SYSCALL) {
        printf("[AMD64] SYSCALL/SYSRET available\n");
    }
    if (edx & AMD_FEATURE_1GB_PAGE) {
        printf("[AMD64] 1GB page support available\n");
    }
    if (ecx & AMD_FEATURE_SVM) {
        printf("[AMD64] AMD-V virtualization available\n");
    }
}

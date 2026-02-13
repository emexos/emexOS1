#pragma once

#define KERNEL_DEFNAME "emexOS [x86-64] "
#define KERNEL_BARENAME "emexOS"
#define KERNEL_DEFRELEASE "v0.5"
#define KERNEL_DEFRELEASE_NAME " v26J5 "
#define KERNEL_VERSION KERNEL_BARENAME " " KERNEL_DEFRELEASE
#define SYSTEMINI_PATH "/emr/config/system.ini"

#define USE_HCF 1
#define BOOTUP_VISUALS 0 // verbose boot == 0, silent boot == 1

// 1 == run tests like processes, scheduler which are in early developement and not finished
// 0 == disable running those tests
#define RUNTESTS 1

// 1 == Hardware compatibility on
// 0 == Hardware compatibility off
// NOTE:
// on some hardware you can use "hardware compatibility off" and it will still run
#define HARDWARE_SC 0

// 1 == enable automatic formatting
// 0 == require manual formatting
// NOTE: this will ERASE *ALL DATA* on your disk if enabled!
#define OVERWRITEALL 0

#if HARDWARE_SC == 1
    #define ENABLE_FAT32 0
    #define ENABLE_ATA 0
    #define ENABLE_ULIME 0
    #define ENABLE_GLIME 0
#else
    #define ENABLE_FAT32 1
    #define ENABLE_ATA 1
    #define ENABLE_ULIME 1
    #define ENABLE_GLIME 1
#endif

#define X64 1
#define RISCV 0
#define ARM64 0

// Import builtin module for target-specific information
#include <types.h>
#include "../console.h"

/* -------------------------------------------------------------------------------------------------------
 *
 * from https: //codeberg.org/warposteam/warpos-src/src/branch/warp/source/monolithic/shell/poweroff.zig#
 * converted to c
 *
 * -------------------------------------------------------------------------------------------------------
 */

/// x86_64 specific poweroff implementation using QEMU's shutdown mechanism
/// Writes to the QEMU-specific I/O port to trigger system shutdown
static inline void x86_poweroff(void) {
    // Single assembly instruction for QEMU shutdown
    __asm__ volatile (
        // Write 16-bit value to I/O port to trigger shutdown
        // outw %ax, %dx - Output word from AX register to port in DX register
        "outw %0, %1"
        :
        // Input operands:
        // [value] - Shutdown command (0x2000) loaded into AX register
        // [port]  - QEMU shutdown port (0x604) loaded into DX register
        : "a" ((u16)0x2000),
        "d" ((u16)0x604)
        // Clobber list: inform compiler about memory modifications
        : "memory"
    );
}

/// System poweroff function - architecture-independent entry point
/// This function is exported for use by other languages and system components
__attribute__((visibility("default")))
void poweroff(void) {
    // Dispatch to architecture-specific implementation
#if defined(__x86_64__)
    x86_poweroff(); // x86_64 architecture
#else
    #error "Unsupported architecture" // Compile-time error for unsupported arch
    //panic();
#endif
}


FHDR(cmd_poweroff) {
    (void)s;
    // Einfach Poweroff aufrufen
    poweroff();
}

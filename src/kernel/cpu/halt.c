#include "cpu.h"

//
// MADE BY @msaid5860
//

// Disable interrupts
void cli(void) {
    __asm__ volatile("cli");
}

// Enable interrupts
void sti(void) {
    __asm__ volatile("sti");
}

// Full system halt
__attribute__((noreturn)) void chalt(void) {
    __asm__ volatile("cli");
    for (;;) __asm__ volatile("hlt");
}

void halt(void) {
    __asm__ volatile("hlt");
}

// Idle halt
__attribute__((noreturn)) void idle(void) {
    for (;;) __asm__ volatile("hlt");
}

// Wait for interrupt
void wfi(void) {
    __asm__ volatile("sti; pause; hlt");
}

void nop(void) {
	__asm__ volatile("nop");
}
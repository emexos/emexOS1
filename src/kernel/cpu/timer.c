#include "timer.h"
#include "irq.h"
#include <kernel/proc/scheduler.h>

static volatile u64 timer_ticks = 0;
static volatile int timer_initialized = 0;

//TODO: create ports.c/.h
//TODO: implement apic/tsc/pic
static inline void outb(u16 port, u8 val)
{
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline void io_wait(void)
{
    // Short delay for I/O operations
    __asm__ volatile("outb %%al, $0x80" : : "a"(0));
}

void timer_handler(cpu_state_t* state)
{
    (void)state;

    if (!timer_initialized) {
        return;
    }

    timer_ticks++;

    // call schedule
    if (sched_is_enabled()) {
        sched_tick();
    }
    //schedule not really implemented
}

void timer_init(u32 frequency)
{
    if (frequency == 0 || frequency > 1193182) {
        frequency = TIMER_FREQUENCY;
    }

    // regist Timer IRQ Handler
    irq_register_handler(0, timer_handler);

    // for PIT
    u32 divisor = 1193182 / frequency;

    if (divisor < 1) divisor = 1;
    if (divisor > 65535) divisor = 65535;

    // send command byte (Channel 0, lo/hi byte, rate generator)
    outb(0x43, 0x36);
    io_wait();

    // Sende Frequency Divisor
    outb(0x40, divisor & 0xFF);         // Low byte
    io_wait();
    outb(0x40, (divisor >> 8) & 0xFF);  // High byte
    io_wait();

    timer_ticks = 0;
    timer_initialized = 1;
}

void timer_wait(u32 ticks)
{
    if (!timer_initialized || ticks == 0) {
        return;
    }

    u64 start = timer_ticks;
    while ((timer_ticks - start) < ticks) {
        __asm__ volatile("hlt");
    }
}

u64 timer_get_ticks(void){
    return timer_ticks;
}

u64 timer_get_seconds(void)
{
    if (!timer_initialized) {
        return 0;
    }
    return timer_ticks / TIMER_FREQUENCY;
}

u64 timer_get_milliseconds(void)
{
    if (!timer_initialized) {
        return 0;
    }
    return timer_ticks;
}

void timer_handler_internal(void)
{
    if (!timer_initialized) {
        return;
    }

    timer_ticks++;

    if (sched_is_enabled()) {
        sched_tick();
    }
}

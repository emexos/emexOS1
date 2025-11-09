#include "timer.h"
#include "irq.h"
#include <kernel/include/ports.h>
#include <kernel/proc/scheduler.h>

static volatile u64 timer_ticks = 0;
static volatile int timer_initialized = 0;

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
}

void timer_init(u32 frequency)
{
    if (frequency == 0 || frequency > 1193182) {
        frequency = TIMER_FREQUENCY;
    }

    // register Timer IRQ Handler
    irq_register_handler(0, timer_handler);

    // for PIT
    u32 divisor = 1193182 / frequency;

    if (divisor < 1) divisor = 1;
    if (divisor > 65535) divisor = 65535;

    // send command byte (Channel 0, lo/hi byte, rate generator)
    outb(0x43, 0x36);
    io_wait();

    // send frequency divisor
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

u64 timer_get_ticks(void)
{
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

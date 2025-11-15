#include "timer.h"
#include "irq.h"
#include <kernel/include/ports.h>
#include <kernel/proc/scheduler.h>
#include <theme/stdclrs.h>
#include <klib/string/string.h>
#include <klib/string/print.h>
#include <drivers/cmos/cmos.h>

// using PIT (8254)
//
static volatile u64 timer_ticks = 0;
static volatile int timer_initialized = 0;
static u64 boot_timestamp = 0;

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
        //TODO:
        // the frequency could be moved to theme/doccr.h or in the os to esr/
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

    // Enable interrupts to start timer
    __asm__ volatile("sti");
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

void timer_set_boot_time(void) {
    boot_timestamp = timer_ticks;
}

u64 timer_get_uptime_seconds(void)
{
    if (!timer_initialized) {
        return 0;
    }
    // Uptime seit timer_set_boot_time()
    u64 current_ticks = timer_ticks - boot_timestamp;
    return current_ticks / TIMER_FREQUENCY;
}

void timer_print_uptime(void)
{
    // for debug
    /*char d[128];
    str_copy(d, "ticks=");
    str_append_uint(debug, (u32)timer_ticks);
    str_append(d, ",boot=");
    str_append_uint(debug, (u32)boot_timestamp);
    str_append(d, ",diff=");
    str_append_uint(d, (u32)(timer_ticks - boot_timestamp));
    print(d, GFX_WHITE;
    print("\n", GFX_WHITE);*/

    u64 uptime = timer_get_uptime_seconds();

    //not really good because its limited
    u64 days = uptime / 86400;
    u64 hours = (uptime % 86400) / 3600;
    u64 minutes = (uptime % 3600) / 60;
    u64 seconds = uptime % 60;

    char buf[128];

    if (days > 0) {
        str_copy(buf, "");
        str_append_uint(buf, (u32)days);
        str_append(buf, " day");
        if (days != 1) str_append(buf, "s");
        str_append(buf, ", ");
        print(buf, GFX_WHITE);
    }

    //TODO:
    // move the function to console/functions/time.c with cmos
    str_copy(buf, "");
    str_append_uint(buf, (u32)hours);
    str_append(buf, ":");
    if (minutes < 10) str_append(buf, "0");
    str_append_uint(buf, (u32)minutes);
    str_append(buf, ":");
    if (seconds < 10) str_append(buf, "0");
    str_append_uint(buf, (u32)seconds);
    print(buf, GFX_WHITE);
}

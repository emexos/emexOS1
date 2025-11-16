#ifndef TIMER_H
#define TIMER_H

#include <types.h>

#define TIMER_FREQUENCY 1000  // 1000 Hz = 1ms Ticks
#define MAX_TIMER_CALLBACKS 8

//callback type
typedef void (*timer_callback_t)(void);

void timer_init(u32 frequency);
void timer_wait(u32 ticks);
u64 timer_get_ticks(void);
u64 timer_get_seconds(void);
u64 timer_get_milliseconds(void);
int timer_register_callback(timer_callback_t callback);
void timer_unregister_callback(timer_callback_t callback);

// for  uptime command
void timer_set_boot_time(void);
u64 timer_get_uptime_seconds(void);
void timer_print_uptime(void);

#endif

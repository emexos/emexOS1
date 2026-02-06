#include <kernel/console/console.h>
#include <drivers/cmos/cmos.h>
#include <kernel/arch/x64/exceptions/timer.h>

FHDR(cmd_cal) {
    (void)s;

    GetCMOSDate();
    print(" ", GFX_WHITE);
    GetCMOSTime();

}

FHDR(cmd_date) {
    (void)s;

    GetCMOSDate();
}

FHDR(cmd_time) {
    (void)s;

    GetCMOSTime();
}

FHDR(cmd_uptime) {
    (void)s;

    print("Uptime: ", GFX_WHITE);
    timer_print_uptime();
}

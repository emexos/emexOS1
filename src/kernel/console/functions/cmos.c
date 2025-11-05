#include <kernel/console/console.h>
#include <drivers/cmos/cmos.h>

FHDR(cmd_date) {
    (void)s;
    GetCMOSDate();
    print("\n", GFX_WHITE);
}

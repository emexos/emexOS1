#include "../console.h"
#include "../../../libs/print/print.h"
#include "../../../libs/graphics/graphics.h"
#include "../../../drivers/cmos/cmos.h"

FHDR(cmd_date) {
    (void)s;

    GetCMOSDate();
    print("\n", GFX_WHITE);

}

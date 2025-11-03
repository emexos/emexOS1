#include "../console.h"
#include "../../../klib/string/print.h"
#include "../../../klib/graphics/graphics.h"
#include "../../../drivers/cmos/cmos.h"
#include "../../../../shared/theme/stdclrs.h"

FHDR(cmd_date) {
    (void)s;

    GetCMOSDate();
    print("\n", GFX_WHITE);

}

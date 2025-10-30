#include "../console.h"
#include "../../../libs/print/print.h"
#include "../../../libs/graphics/graphics.h"
#include "../../../libs/string/string.h"
#include "../../../libs/memory/main.h"
#include "../../../drivers/cmos/cmos.h"


FHDR(cmd_meminfo)
{
    (void)s; // unused parameter

    char buf[128];

    print("\n", GFX_WHITE);

    str_copy(buf, "Free:  ");
    str_append_uint(buf, (u32)(mem_get_free() / 1024));
    str_append(buf, " KB");
    print(buf, GFX_GREEN);

    str_copy(buf, "\nUsed:  ");
    str_append_uint(buf, (u32)(mem_get_used() / 1024));
    str_append(buf, " KB");
    print(buf, GFX_YELLOW);

    size_t total = mem_get_free() + mem_get_used();
    str_copy(buf, "\nTotal: ");
    str_append_uint(buf, (u32)(total / 1024));
    str_append(buf, " KB");
    print(buf, GFX_WHITE);

    print("\n", GFX_WHITE);
}

FHDR(cmd_sysinfo)
{
    (void)s; // unused parameter

    print("\n", GFX_WHITE);
    print("                        \n", GFX_GREEN);
    print("   ###########;m;       user@doccros\n", GFX_GREEN);
    print("   # #########;m;       ----------------\n", GFX_GREEN);
    print("   # #;m;               OS: ", GFX_GREEN); print("doccrOS x86 \n", GFX_WHITE);
    print("   # #;m;               version: ", GFX_GREEN); print("pav0.1\n", GFX_WHITE);
    print("   # ########;m;        Bootloader: ", GFX_GREEN); print("Limine \n", GFX_WHITE);
    print("   # ########;m;        Resolution: ", GFX_GREEN); print("1280x800\n", GFX_WHITE);
    print("   # #;m;               CPU: ", GFX_GREEN); /*ShowCPUName();*/ print("not detected\n", GFX_WHITE);
    print("   # #;m;               Date: ", GFX_GREEN); GetCMOSDate(); print("\n", GFX_WHITE);
    print("   # #########;m;       graphic: ", GFX_GREEN); print("not detected\n", GFX_WHITE); //should show like nvidia or something like video card
    print("   ###########;m;       \n", GFX_GREEN);
    print("                        ", GFX_GREEN);
    print("\n", GFX_WHITE);


    /*print("\n  --------------------------------------------------------------------", 0x0F);
    print("\n   Developer: ", 0x0A); Print("/salsaJon\n", 0x0F);
    print("   First gfx Build: ", 0x0A); Print("05/04/2025\n\n", 0x0F);

    print("   This OS is made for a school Project, its open source \n", 0x0F);
    print("   Also, emexOS is the most hardest and best project i made.\n", 0x0F);
    print("   this is my first OS with a GUI\n\n", 0x0F);

    print("\n  --------------------------------------------------------------------", 0x0F);*/
}

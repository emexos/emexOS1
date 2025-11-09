#include <kernel/console/console.h>
#include <klib/memory/main.h>
#include <drivers/cmos/cmos.h>
#include <kernel/module/module.h>

FHDR(cmd_modules)
{
    (void)s; // unused
    print("Loaded Modules:\n", GFX_WHITE);

    int count = module_get_count();

    if (count == 0) {
        print("No modules loaded\n", GFX_RED);
        return;
    }

    char buf[128];
    for (int i = 0; i < count; i++) {
        driver_module *mod = module_get_by_index(i);
        if (mod) {

            //print("- ", GFX_WHITE);
            //print(mod->name, GFX_GREEN);

            //print(" -> ", GFX_WHITE);
            print(mod->mount, GFX_CYAN);

            //i simplified the module printing because its uneccesary

            u32 ver = mod->version;
            u32 major = (ver >> 24) & 0xFF;
            u32 minor = (ver >> 16) & 0xFF;
            u32 patch = (ver >> 8) & 0xFF;

            //actually version is uneccesary too but i let it for now idk
            str_copy(buf, " [v");
            str_append_uint(buf, major);
            str_append(buf, ".");
            str_append_uint(buf, minor);
            str_append(buf, ".");
            str_append_uint(buf, patch);
            str_append(buf, "]");
            print(buf, GFX_GRAY_50);

            print("\n", GFX_WHITE);
        }
    }

    str_copy(buf, "\nTotal: ");
    str_append_uint(buf, count);
    str_append(buf, " module(s)");
    print(buf, GFX_WHITE);
    //print("\nkeep in mind it will only show all modules if the os has a fs/vfs/memfs", GFX_GRAY_20);
}

FHDR(cmd_meminfo)
{
    (void)s; // unused parameter

    char buf[128];

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

}

FHDR(cmd_sysinfo)
{
    (void)s; // unused parameter

    print("                    \n", GFX_GREEN);
    print(" ###########;m;      user@emexos\n", GFX_GREEN);
    print(" # #########;m;      \x02\x02\x02\x02\x02\x02\x02\x02\x02\x02\x02\x02\x02\x02\x02\n", GFX_GREEN); //15 characters
    print(" # #;m;              Kernel: ", GFX_GREEN); print("emexOS x86 \n", GFX_WHITE);
    print(" # #;m;              Version: ", GFX_GREEN); print("v2.1 [64bit]\n", GFX_WHITE);
    print(" # ########;m;       Bootloader: ", GFX_GREEN); print("Limine \n", GFX_WHITE);
    print(" # ########;m;       Resolution: ", GFX_GREEN); print("1280x800\n", GFX_WHITE);
    print(" # #;m;              CPU: ", GFX_GREEN); /*ShowCPUName();*/ print("not detected\n", GFX_WHITE);
    print(" # #;m;              Date: ", GFX_GREEN); GetCMOSDate(); print("\n", GFX_WHITE);
    //print(" # #########;m;      graphic: ", GFX_GREEN); print("not detected\n", GFX_WHITE); //should show like nvidia or something like video card
    print(" # #########;m;      \x02\x02\x02\x02\x02\x02\x02\x02\x02\x02\x02\x02\x02\x02\x02\n", GFX_GREEN);
    print(" ###########;m;      ", GFX_GREEN);
        print("\x09 ", GFX_WHITE); //all colors:
        print("\x09 ", GFX_RED);
        print("\x09 ", GFX_GREEN);
        print("\x09 ", GFX_YELLOW);
        print("\x09 ", GFX_BLUE);
        print("\x09 ", GFX_PURPLE);
        print("\x09 ", GFX_CYAN);
        print("\x09 \n", GFX_BG); //i think nobody will see this xd
    print("                        ", GFX_GREEN); // empty/new line

}

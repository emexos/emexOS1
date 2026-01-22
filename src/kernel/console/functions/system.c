#include <kernel/console/console.h>
#include <memory/main.h>
#include <drivers/cmos/cmos.h>
#include <kernel/module/module.h>
#include <kernel/exceptions/timer.h>
#include <kernel/cpu/cpu.h>

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
    // (void)s; // unused parameter

    // char buf[128];

    // u64 free = mem_get_free() / 1024;
    // u64 used = mem_get_used() / 1024;
    // u64 total = mem_get_total() / 1024;

    // str_copy(buf, "Free :  ");
    // str_append_uint(buf, (u32)(free >> 32));
    // str_append(buf, ":");
    // str_append_uint(buf, (u32)(free));
    // str_append(buf, " KB");
    // print(buf, GFX_GREEN);

    // str_copy(buf, "\nUsed :  ");
    // str_append_uint(buf, (u32)(used >> 32));
    // str_append(buf, ":");
    // str_append_uint(buf, (u32)(used));
    // str_append(buf, " KB");
    // print(buf, GFX_YELLOW);

    // str_copy(buf, "\nTotal:  ");
    // str_append_uint(buf, (u32)(total >> 32));
    // str_append(buf, ":");
    // str_append_uint(buf, (u32)(total));
    // str_append(buf, " KB");
    // print(buf, GFX_WHITE);

}

void print_res() {
    char buf[128];
    str_copy(buf, "");
    str_append_uint(buf, get_fb_width());
    str_append(buf, "x");
    str_append_uint(buf, get_fb_height());
    str_append(buf, "\n");
    print(buf, GFX_WHITE);
}

void ShowCPUName(){
    const char *cpu_name = cpu_get_brand();
    if (cpu_name[0]) {
        print(cpu_name, GFX_WHITE);
    } else {
        print("Unknown CPU", GFX_WHITE);
    }
}

FHDR(cmd_sysinfo)
{
    (void)s; // unused parameter

    print("                  \n", GFX_GREEN);
    print(" ###########;m;    ", GFX_GREEN); print(PC_NAME, GFX_GREEN); print("@", GFX_GREEN); print(USER_NAME, GFX_GREEN); print("\n", GFX_GREEN);
    print(" # #########;m;    \x02\x02\x02\x02\x02\x02\x02\x02\x02\x02\x02\x02\x02\x02\x02\x02\n", GFX_GREEN); //15 characters
    print(" # #;m;            Kernel: ", GFX_GREEN); print(OS_DEFNAME, GFX_WHITE); print(OS_DEFRELEASE, GFX_WHITE); print("\n", GFX_WHITE);
    print(" # #;m;            Resolution: ", GFX_GREEN); print_res();
    print(" # ########;m;     Bootloader: ", GFX_GREEN); print("Limine \n", GFX_WHITE);
    print(" # ########;m;     CPU: ", GFX_GREEN); ShowCPUName(); print("\n", GFX_WHITE);
    print(" # #;m;            Date: ", GFX_GREEN); GetCMOSDate(); print("\n", GFX_WHITE);
    print(" # #;m;            Uptime: ", GFX_GREEN); timer_print_uptime(); print("\n", GFX_WHITE);
    print(" # #########;m;    \x02\x02\x02\x02\x02\x02\x02\x02\x02\x02\x02\x02\x02\x02\x02\x02\n", GFX_GREEN);
    print(" ###########;m;    ", GFX_GREEN);
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

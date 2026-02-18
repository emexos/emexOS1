#include <kernel/console/console.h>
#include <memory/main.h>
#include <drivers/cmos/cmos.h>
#include <kernel/module/module.h>
#include <kernel/arch/x86_64/exceptions/timer.h>
#include <kernel/cpu/cpu.h>

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
    cprintf(buf, GFX_WHITE);
}

void ShowCPUName(){
    const char *cpu_name = cpu_get_brand();
    if (cpu_name[0]) {
        cprintf(cpu_name, GFX_WHITE);
    } else {
        cprintf("Unknown CPU", GFX_WHITE);
    }
}

FHDR(cmd_sysinfo)
{
    (void)s; // unused parameter

    cprintf("                  \n", GFX_GREEN);
    cprintf(" ###########;m;    ", GFX_GREEN); cprintf(PC_NAME, GFX_GREEN); cprintf("@", GFX_GREEN); cprintf(USER_NAME, GFX_GREEN); cprintf("\n", GFX_GREEN);
    cprintf(" # #########;m;    ---------------\n", GFX_GREEN); //15 characters
    cprintf(" # #;m;            Kernel: ", GFX_GREEN); cprintf(KERNEL_BARENAME " " KERNEL_DEFRELEASE, GFX_WHITE); cprintf("\n", GFX_WHITE);
    cprintf(" # #;m;            Resolution: ", GFX_GREEN); print_res();
    cprintf(" # ########;m;     Bootloader: ", GFX_GREEN); cprintf("Limine \n", GFX_WHITE);
    cprintf(" # ########;m;     CPU: ", GFX_GREEN); ShowCPUName(); cprintf("\n", GFX_WHITE);
    cprintf(" # #;m;            Date: ", GFX_GREEN); GetCMOSDate(); cprintf("\n", GFX_WHITE);
    cprintf(" # #;m;            Uptime: ", GFX_GREEN); timer_print_uptime(); cprintf("\n", GFX_WHITE);
    cprintf(" # #########;m;    ---------------\n", GFX_GREEN);
    cprintf(" ###########;m;    ", GFX_GREEN);
        cprintf("^ ", GFX_WHITE); //all colors:
        cprintf("^ ", GFX_RED);
        cprintf("^ ", GFX_GREEN);
        cprintf("^ ", GFX_YELLOW);
        cprintf("^ ", GFX_BLUE);
        cprintf("^ ", GFX_PURPLE);
        cprintf("^ ", GFX_CYAN);
        cprintf("^ \n", GFX_BG); //i think nobody will see this xd
    cprintf("                        ", GFX_GREEN); // empty/new line

}

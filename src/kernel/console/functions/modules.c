#include <kernel/console/console.h>
#include <memory/main.h>
#include <kernel/module/module.h>

FHDR(cmd_modules)
{
    (void)s; // unused
    cprintf("Loaded Modules:\n", GFX_WHITE);

    int count = module_get_count();

    if (count == 0) {
        cprintf("No modules loaded\n", GFX_WHITE);
        return;
    }

    char buf[128];
    for (int i = 0; i < count; i++) {
        driver_module *mod = module_get_by_index(i);
        if (mod) {

            //cprintf("- ", GFX_WHITE);
            //cprintf(mod->name, GFX_GREEN);

            //cprintf(" -> ", GFX_WHITE);
            cprintf(mod->mount, GFX_WHITE);

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
            cprintf(buf, GFX_GRAY_50);

            cprintf("\n", GFX_WHITE);
        }
    }

    str_copy(buf, "\nTotal: ");
    str_append_uint(buf, count);
    str_append(buf, " module(s)");
    cprintf(buf, GFX_WHITE);
    //cprintf("\nkeep in mind it will only show all modules if the os has a fs/vfs/memfs", GFX_GRAY_20);
}

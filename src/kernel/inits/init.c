#include <limine/limine.h>
//#include <config/paths.h>
#include <kernel/file_systems/vfs/vfs.h>
#include <kernel/communication/serial.h>
#include <kernel/modules/limine.h>
#include <kernel/graph/theme.h>
#include <theme/doccr.h>

void keymaps_load(void)
{

    // create keymap settings
    int fd_keymap_cfg = fs_open("/emr/config/keymaps/keymap.cfg", O_CREAT | O_WRONLY);
    if (fd_keymap_cfg >= 0) {
        const char *default_config = "# US, PL, DE \nKEYMAP: US\n";
        fs_write(fd_keymap_cfg, default_config, str_len(default_config));
        fs_close(fd_keymap_cfg);
    }

    BOOTUP_PRINT("[FS] ", GFX_GRAY_70);
    BOOTUP_PRINT("created keymap.cfg\n", white());

    // load all keymaps
    limine_module_load("US.map", "/emr/config/keymaps/US.map");
    limine_module_load("DE.map", "/emr/config/keymaps/DE.map");
    limine_module_load("PL.map", "/emr/config/keymaps/PL.map");

}

void logos_load(void) {
    limine_module_load("logo.bin", "/boot/ui/assets/bootlogo.bin");
    limine_module_load("logo.bmp", "/emr/assets/logo.bmp");
    //limine_module_load("console_icon.bmp", "/images/iconsole.bmp");
    //limine_module_load("desktop_icon.bmp", "/images/idesktop.bmp");
    limine_module_load("background.bmp", "/images/bg.bmp");
}

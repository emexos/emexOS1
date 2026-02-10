#include <limine/limine.h>
//#include <config/paths.h>
#include <kernel/file_systems/vfs/vfs.h>
#include <kernel/communication/serial.h>
#include <kernel/modules/limine.h>
#include <kernel/graph/theme.h>
#include <theme/doccr.h>

#include <config/user.h>
#include <config/system.h>

void keymaps_load(void)
{

    // create keymap settings
    int fd_keymap_cfg = fs_open("/emr/config/keymaps/keymap.cfg", O_CREAT | O_WRONLY);
    if (fd_keymap_cfg >= 0) {
        const char *default_config = "# US, PL, DE \nKEYMAP: US\n";
        fs_write(fd_keymap_cfg, default_config, str_len(default_config));
        fs_close(fd_keymap_cfg);
    }

    log("[FS]","created keymap.cfg\n", d);

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

void users_load(void) {
    int fd_users = fs_open(USERINI_PATH  "", O_CREAT | O_WRONLY); // how can this be possible.....
    int fd_system = fs_open(SYSTEMINI_PATH "",  O_CREAT | O_WRONLY); // if it works don't touch it!

    if (fd_users >= 0) {
        const char *users_config =
            "[USERS]\n"
            "users=admin,user\n\n"
            "[admin]\n"
            "permissions=administrator\n\n"
            "[emex]\n"
            "permissions=user\n";
        fs_write(fd_users, users_config, str_len(users_config));
        fs_close(fd_users);
    }
    if (fd_system >= 0) {
        const char *system_config =
            "[GENERAL]\n"
            "default_shell=/user/bin/shell.elf\n"
            "root_user=admin # from users.ini\n";
        fs_write(fd_system, system_config, str_len(system_config));
        fs_close(fd_system);
    }
    fs_mkdir("/users");
    fs_mkdir("/users/bin");
    fs_mkdir("/users/apps");
}

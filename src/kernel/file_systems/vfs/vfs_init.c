// src/kernel/file_systems/vfs/fs_init.c

#include "vfs.h"
#include <kernel/mem/klime/klime.h>
#include <kernel/module/module.h>

#include <klib/string/string.h>
#include <theme/doccr.h>

/*void fs_check_bootconf(void) {
    int fd = fs_open("/boot/bootconf", O_RDONLY);
    if(fd < 0) {
        BOOTUP_PRINT("[FS] ", GFX_GRAY_70);
        BOOTUP_PRINT("bootconf not found\n", GFX_ST_RED);
        return;
    }

    char buf[128];
    int n;
    while((n = fs_read(fd, buf, sizeof(buf)-1)) > 0) {
        buf[n] = 0;
        BOOTUP_PRINT(buf, GFX_ST_WHITE);
    }

    fs_close(fd);
    BOOTUP_PRINT("[FS] ", GFX_GRAY_70);
    BOOTUP_PRINT("bootconf loaded\n", GFX_ST_GREEN);
}


void fs_load_bootconf(void) {
    const char *src = "/boot/bootconf"; // mount frm iso
    const char *dst = "/tmp/bootconf";

    int fd_src = fs_open(src, O_RDONLY);
    if(fd_src < 0) return;

    int fd_dst = fs_open(dst, O_CREAT | O_WRONLY);
    if(fd_dst < 0) {
        fs_close(fd_src);
        return;
    }

    char buf[512];
    int n;
    while((n = fs_read(fd_src, buf, sizeof(buf))) > 0) {
        fs_write(fd_dst, buf, n);
    }

    fs_close(fd_src);
    fs_close(fd_dst);
}
*/

void fs_system_init(void *klime)
{
    // set klime for all fs types (l: except diskfs's)
    fs_klime = klime;
    //fs_set_klime(klime);
    //tmpfs_set_klime(klime);
    //devfs_set_klime(klime);

    // init core
    fs_init();

    // register fs types
    tmpfs_register();
    devfs_register();

    BOOTUP_PRINT("[FS] ", GFX_GRAY_70);
    BOOTUP_PRINT("mounting roots: \n", GFX_ST_WHITE);

    // mount root as tmpfs
    fs_mount(NULL, "/", "tmpfs"); // in future its just root so for (fat32,ext2,...)
    //fs_mount(NULL, "/", "/"); // or root

    // create standard dirs
    fs_mkdir("/dev");
    fs_mkdir("/tmp");
    fs_mkdir("/boot");
    //TODO: bootconf loading from /boot/bootconf
    // in shared/theme/bootconf file

    //fs_check_bootconf();
    //fs_load_bootconf();

    // mount devfs at /dev
    fs_mount(NULL, "/dev", "devfs");

}

void fs_register_mods()
{
    int total = module_get_count();
    int dev_cnt = 0; // device count

    BOOTUP_PRINT("[FS] ", GFX_GRAY_70);
    BOOTUP_PRINT("scann modules:\n", GFX_ST_WHITE);

    for (int i = 0; i < total; i++) {
        driver_module *mod = module_get_by_index(i);

        // only registers modules with /dev/* mount paths
        // these are device drivers that need devfs nodes
        if (mod && mod->mount && str_starts_with(mod->mount, "/dev/")) {
            //printf("reg to: %s\n", mod->name);
            if (devfs_register_device(mod) == 0) {
                dev_cnt++;
            }
        }
    }

    BOOTUP_PRINT("[FS] ", GFX_GRAY_70);
    BOOTUP_PRINT("registered ", GFX_ST_WHITE);
    BOOTUP_PRINT_INT(dev_cnt, GFX_ST_YELLOW); // device count
    BOOTUP_PRINT(" device(s)\n", GFX_ST_WHITE);
}

void fs_create_test_file(void) {
    // create and write test file in tmpfs
    // Write only
    int fd = fs_open("/tmp/t", O_CREAT | O_WRONLY);
    if (fd < 0) {
        BOOTUP_PRINT("[FS] ", GFX_GRAY_70);
        BOOTUP_PRINT("failed to create file\n", GFX_ST_WHITE);
        return;
    }

    fs_write(fd, "this is a test for fs this file has no other use", str_len("this is a test for fs this file has no other use"));
    fs_close(fd);

    BOOTUP_PRINT("[FS] ", GFX_GRAY_70);
    BOOTUP_PRINT("created ", GFX_ST_WHITE);
    BOOTUP_PRINT("/tmp/t", GFX_ST_CYAN);
    BOOTUP_PRINT("\n", GFX_ST_WHITE);
}

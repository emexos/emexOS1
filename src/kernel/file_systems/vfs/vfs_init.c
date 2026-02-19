#include "init.h"
int init_boot_log = -1;
char *logpath = "/emr/logs/log1.txt";
#include "vfs.h"
#include <kernel/mem/klime/klime.h>
#include <kernel/include/reqs.h>
#include <kernel/module/module.h>
#include <kernel/arch/x86_64/exceptions/panic.h>

#include <string/string.h>
#include <kernel/graph/theme.h>
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

static void load_limine_module(void) {
    if (!module_request.response || module_request.response->module_count == 0) {
        log("[FS]", "no limine modules to copy\n", warning);
        return;
    }

    struct limine_module_response *response = (struct limine_module_response *)module_request.response;

    log("[FS]", "setting UI config...\n", d);
    fs_mkdir("/boot/ui");
    fs_mkdir("/boot/ui/assets");

    log("[FS]", "copying limine modules to VFS:\n", d);

    for (u64 i = 0; i < response->module_count; i++) {
        char buf[32];
        struct limine_file *module = response->modules[i];

        // extract filename (e.g., "logo.bin" from "/boot/ui/.../logo.bin")
        const char *filename = module->path;
        const char *last_slash = filename;
        for (const char *p = filename; *p; p++) {
            if (*p == '/') last_slash = p + 1;
        }
        filename = last_slash;

        // Create full path in VFS
        char vfs_path[256];
        str_copy(vfs_path, LOGO_NAME);
        //str_append(vfs_path, "/");
        //str_append(vfs_path, LOGO_NAME); // == /boot/ui/assets/logo.bin
        //str_append(vfs_path, filename);



        int fd = fs_open(vfs_path, O_CREAT | O_WRONLY);
        if (fd < 0) {
            log("[FS]", "ERROR: cannot create ", warning);
            BOOTUP_PRINT(vfs_path, white());
            BOOTUP_PRINT("\n", white());
            continue;
        }

        ssize_t written = fs_write(fd, (void*)module->address, module->size);
        fs_close(fd);

        if (written > 0) {
            BOOTUP_PRINT("     ", white());
            BOOTUP_PRINT(module->path, white());
            BOOTUP_PRINT(" : ", white());
            BOOTUP_PRINT(vfs_path, white());
            str_copy(buf, " with ");
            str_append_uint(buf, (u32)written);
            str_append(buf, " bytes\n");
            BOOTUP_PRINT(buf,  white());
        }
    }
}

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

    log("[FS]", "mounting roots: \n", white());

    // mount root as tmpfs
    fs_mount(NULL, ROOT_MOUNT_DEFAULT, ROOTFS); // in future its just root so for (fat32,ext2,...)
    //fs_mount(NULL, "/", "/"); // or root

    // only necessary dirs the rest comes from initrd.cpio
    // create standard dirs
    fs_mkdir(DEV_DIRECTORY);
    fs_mkdir(TMP_DIRECTORY);

    fs_mkdir(BOOT_DIRECTORY);
    //emx system requirement paths
    fs_mkdir(EMX_DIRECTORY); // /emr
    fs_mkdir(EMLOG_DIRECTORY);
    //fs_mkdir(EMAST_DIRECTORY);
    //fs_mkdir(EMCFG_DIRECTORY);
    //fs_mkdir(KEYMP_DIRECTORY);
    //fs_mkdir(EMDRV_DIRECTORY);

    //fs_mkdir(CONF_DIRECTORY);


    init_boot_log = fs_open(logpath, O_CREAT | O_WRONLY);
    log("[FS]", "[FS] wrote", d);
    BOOTUP_PRINT(logpath, white());
    if (init_boot_log < 0) {
        panic("Cannot open [logs]");
    }
    BOOTUP_PRINT("\n", white());
    //TODO: bootconf loading from /boot/bootconf
    // in shared/theme/bootconf file

    //fs_check_bootconf();
    //fs_load_bootconf();

    // mount devfs at /dev
    fs_mount(NULL, DEV_MOUNT_DEFAULT, DEVFS);

    //last thing for init is loading all limine modules
    load_limine_module(); // logo

}

void fs_register_mods()
{
    int total = module_get_count();
    int dev_cnt = 0; // device count

    log("[FS]", "scann modules:\n", d);

    for (int i = 0; i < total; i++) {
        driver_module *mod = module_get_by_index(i);

        // only registers modules with /dev/* mount paths
        // these are device drivers that need devfs nodes
        if (mod && mod->mount && str_starts_with(mod->mount, _DEV)) {
            //printf("reg to: %s\n", mod->name);
            if (devfs_register_device(mod) == 0) {
                dev_cnt++;
            }
        }
    }

    log("[FS]", "registered ", d);
    BOOTUP_PRINT_INT(dev_cnt, white()); // device count
    BOOTUP_PRINT(" device(s)\n", white());
}

void fs_create_test_file(void) {
    // create and write test file in tmpfs
    // Write only
    int fd = fs_open("/tmp/t", O_CREAT | O_WRONLY);
    if (fd < 0) {
        log("[FS]", "failed to create file\n", error);
        return;
    }

    fs_write(fd, "this is a test for fs this file has no other use", str_len("this is a test for fs this file has no other use"));
    fs_close(fd);

   log("[FS]", "created ", d);
    BOOTUP_PRINT("/tmp/t", white());
    BOOTUP_PRINT("\n", white());
}

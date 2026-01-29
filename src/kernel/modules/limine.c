#include "limine.h"
#include <kernel/include/reqs.h>
#include <kernel/file_systems/vfs/vfs.h>
#include <string/string.h>
#include <memory/main.h>
#include <kernel/mem/klime/klime.h>
#include <kernel/communication/serial.h>
#include <theme/stdclrs.h>
#include <kernel/graph/theme.h>
#include <theme/doccr.h>

extern void *fs_klime;

void limine_modules_init(void) {
    BOOTUP_PRINT("[LIMINE] ", GFX_GRAY_70);
    BOOTUP_PRINT("Checking for Limine modules...\n", white());

    if (!module_request.response || module_request.response->module_count == 0) {
        BOOTUP_PRINT("[LIMINE] ", GFX_GRAY_70);
        BOOTUP_PRINT("No module response available\n", red());
        return;
    }

    struct limine_module_response *response =
        (struct limine_module_response *)module_request.response;

    BOOTUP_PRINT("[LIMINE] ", GFX_GRAY_70);
    BOOTUP_PRINT("Found ", white());
    BOOTUP_PRINT_INT(response->module_count, white());
    BOOTUP_PRINT(" module(s)\n", white());
}

int limine_module_load(const char *module_name, const char *vfs_path) {
    if (!module_request.response || module_request.response->module_count == 0) {
        printf("[LIMINE] No modules available\n");
        return -1;
    }

    struct limine_module_response *response =
        (struct limine_module_response *)module_request.response;

    // searches for module by filename
    struct limine_file *module = NULL;
    for (u64 i = 0; i < response->module_count; i++) {
        const char *path = response->modules[i]->path;

        // extracts filename from path
        const char *filename = path;
        const char *last_slash = path;
        for (const char *p = path; *p; p++) {
            if (*p == '/') last_slash = p + 1;
        }
        filename = last_slash;
        if (str_equals(filename, module_name)) {
            module = response->modules[i];
            break;
        }
    }

    if (!module) {
        BOOTUP_PRINT("[LIMINE] ", GFX_GRAY_70);
        BOOTUP_PRINT("Module not found: ", red());
        BOOTUP_PRINT(module_name, white());
        BOOTUP_PRINT("\n", white());
        return -1;
    }

    int is_tmpfs_path = 0;
    if
    (
        str_starts_with(vfs_path, "/tmp/")     ||
        str_starts_with(vfs_path, "/boot/")    ||
        str_starts_with(vfs_path, "/images/")  ||
        str_starts_with(vfs_path, "/emr/")     ||
        str_starts_with(vfs_path, "/dev/")

    ) {
        is_tmpfs_path = 1;
    }

    // if its not tmpfs cuz fat32 has only read
    if (!is_tmpfs_path) {
        BOOTUP_PRINT("[LIMINE] ", GFX_GRAY_70);
        BOOTUP_PRINT("Skipping write to non-tmpfs path: ", yellow());
        BOOTUP_PRINT(vfs_path, white());
        BOOTUP_PRINT(" (FAT32 is read-only)\n", white());
        return 0;
    }
    int fd = fs_open(vfs_path, O_CREAT | O_WRONLY);
    if (fd < 0) {
        BOOTUP_PRINT("[LIMINE] ", GFX_GRAY_70);
        BOOTUP_PRINT("Cannot create: ", red());
        BOOTUP_PRINT(vfs_path, white());
        BOOTUP_PRINT("\n", white());
        return -1;
    }

    ssize_t written = fs_write(fd, (void*)module->address, module->size);
    fs_close(fd);

    if (written <= 0) {
        BOOTUP_PRINT("[LIMINE] ", GFX_GRAY_70);
        BOOTUP_PRINT("Write failed: ", red());
        BOOTUP_PRINT(vfs_path, white());
        BOOTUP_PRINT("\n", white());
        return -1;
    }

    BOOTUP_PRINT("[LIMINE] ", GFX_GRAY_70);
    BOOTUP_PRINT("Loaded: ", white());
    BOOTUP_PRINT(module_name, cyan());
    BOOTUP_PRINT(" -> ", white());
    BOOTUP_PRINT(vfs_path, white());

    char buf[32];
    str_copy(buf, " (");
    str_append_uint(buf, (u32)written);
    str_append(buf, " bytes)\n");
    BOOTUP_PRINT(buf, white());

    return 0;
}

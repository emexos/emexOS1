#include "limine.h"
#include <kernel/include/reqs.h>
#include <kernel/file_systems/vfs/vfs.h>
#include <kernel/mem/klime/klime.h>
#include <memory/main.h>
#include <string/string.h>
#include <kernel/communication/serial.h>
#include <kernel/graph/theme.h>
#include <theme/doccr.h>

int limine_modules_init(void) {
    if (!module_request.response) {
        BOOTUP_PRINT("[LIMINE] ", GFX_GRAY_70);
        BOOTUP_PRINT("No module response available\n", red());
        return -1;
    }

    struct limine_module_response *response =
        (struct limine_module_response *)module_request.response;

    BOOTUP_PRINT("[LIMINE] ", GFX_GRAY_70);
    BOOTUP_PRINT("Found ", white());
    char buf[32];
    str_copy(buf, "");
    str_append_uint(buf, (u32)response->module_count);
    BOOTUP_PRINT(buf, white());
    BOOTUP_PRINT(" modules\n", white());

    return (int)response->module_count;
}

int limine_modules_count(void) {
    if (!module_request.response) return 0;
    return (int)module_request.response->module_count;
}

struct limine_file* limine_module_get(int index) {
    if (!module_request.response) return NULL;
    if (index < 0 || index >= (int)module_request.response->module_count) {
        return NULL;
    }
    return module_request.response->modules[index];
}

int limine_module_load(const char *source, const char *dest) {
    if (!source || !dest) return -1;

    if (!module_request.response || module_request.response->module_count == 0) {
        printf("[LIMINE] No modules available\n");
        return -1;
    }

    struct limine_module_response *response =
        (struct limine_module_response *)module_request.response;

    // searches for module by filename
    for (u64 i = 0; i < response->module_count; i++) {
        struct limine_file *module = response->modules[i];

        // extracts filename from path
        const char *filename = module->path;
        const char *last_slash = filename;
        for (const char *p = filename; *p; p++) {
            if (*p == '/') last_slash = p + 1;
        }
        filename = last_slash;

        //if this is our module
        if (str_equals(filename, source)) {
            BOOTUP_PRINT("[LIMINE] ", GFX_GRAY_70);
            BOOTUP_PRINT("Loading ", white());
            BOOTUP_PRINT(source, cyan());
            BOOTUP_PRINT(" -> ", white());
            BOOTUP_PRINT(dest, white());

            // it now creates all neccessary dirs automatically so you don't need to do that yourself
            char dir_path[256];
            str_copy(dir_path, dest);

            int last_slash_pos = -1;
            for (int j = 0; dir_path[j]; j++) {
                if (dir_path[j] == '/') last_slash_pos = j;
            }

            if (last_slash_pos > 0) {
                dir_path[last_slash_pos] = '\0';
                // trys to create directory (will fail if exists, that's ok)
                fs_mkdir(dir_path);
            }

            int fd = fs_open(dest, O_CREAT | O_WRONLY);
            if (fd < 0) {
                BOOTUP_PRINT(" FAILED\n", red());
                return -1;
            }

            // writes the data
            ssize_t written = fs_write(fd, (void*)module->address, module->size);
            fs_close(fd);

            if (written > 0) {
                char buf[32];
                str_copy(buf, " (");
                str_append_uint(buf, (u32)written);
                str_append(buf, " bytes)\n");
                BOOTUP_PRINT(buf, green());
                return 0;
            }

            BOOTUP_PRINT(" FAILED\n", red());
            return -1;
        }
    }

    BOOTUP_PRINT("[LIMINE] ", GFX_GRAY_70);
    BOOTUP_PRINT("Module not found: ", red());
    BOOTUP_PRINT(source, white());
    BOOTUP_PRINT("\n", white());
    return -1;
}

#include "emex.h"

#include <kernel/file_systems/vfs/vfs.h>
#include <string/string.h>
#include <kernel/communication/serial.h>


// checks if a given path ends with .emx
static int path_has_emx_ext(const char *path)
{
    if (!path) return 0;

    int len = str_len(path);
    // .emx + 1
    if (len < 5) return 0;

    // check the last 4 characters
    const char *tail = path + len - 4;
    return (tail[0] == '.' &&
            tail[1] == 'e' &&
            tail[2] == 'm' &&
            tail[3] == 'x');
    // guess what... its .emx ... mind blowing ik
}
// builds a full child path by joining base_path + / + child_name
// (/apps/myapp.emx + / + app.elf == /apps/myapp.emx/app.elf)
static void build_child_path(char *out, const char *base, const char *child) {
    str_copy(out, base);

    int blen = str_len(out);
    if (blen > 0 && out[blen - 1] != '/') {
        str_append(out, "/");
    }

    str_append(out, child);
}
static int file_exists(const char *path) {
    fs_node *node = fs_resolve(path);
    if (!node) return 0;
    return (node->type == FS_FILE);
}
static int dir_exists(const char *path) {
    fs_node *node = fs_resolve(path);
    if (!node) return 0;
    return (node->type == FS_DIR);
}

int emex_open_app(const char *path, emex_package_t *pkg) {
    if (!path || !pkg) return EMEX_ERR_NOT_FOUND;

    // wipe the package struct clean before filling it
    str_copy(pkg->path, path);
    pkg->has_icon = 0;
    pkg->has_resources = 0;

    // only if the path ends with .emx
    if (!path_has_emx_ext(path)) {
        printf("[EMX] '%s' doesn't end with .emx, not a valid emx package\n", path);
        return EMEX_ERR_NOT_EMX;
    }
    if (!dir_exists(path)) {
        printf("[EMX] '%s' not found or isn't a directory\n", path);
        return EMEX_ERR_NOT_FOUND;
    }

    printf("[EMX] opening package: %s\n", path);

    char child_path[256];

    // app.elf is required otherwise
    build_child_path(child_path, path, EMEX_ELF_NAME);
    if (!file_exists(child_path)) {
        printf("[EMX] %s is missing! can't launch without it\n", EMEX_ELF_NAME);
        return EMEX_ERR_NO_ELF;
    }
    printf("[EMX] found %s\n", EMEX_ELF_NAME);

    // package.info is also required with title the other things not really but well
    build_child_path(child_path, path, EMEX_INFO_NAME);
    if (!file_exists(child_path)) {
        printf("[EMX] %s is missing! package needs metadata\n", EMEX_INFO_NAME);
        return EMEX_ERR_NO_INFO;
    }
    printf("[EMX] found %s\n", EMEX_INFO_NAME);
    int info_result = emex_parse_info(child_path, &pkg->info);
    if (info_result != EMEX_OK) {
        printf("[EMX] failed to parse %s (code %d)\n", EMEX_INFO_NAME, info_result);
        return EMEX_ERR_BAD_INFO;
    }

    // optional icon
    // icon is later good for a taskbar or smth else but
    // then i probably delete it from the kernel and move the icon parser to the wm or smth
    build_child_path(child_path, path, EMEX_ICON_NAME);
    if (file_exists(child_path)) {
        printf("[EMX] found %s, loading...\n", EMEX_ICON_NAME);
        if (emex_load_icon(child_path, &pkg->icon) == EMEX_OK) {
            pkg->has_icon = 1;
        } else {
            pkg->has_icon = 0;
        }
    } else {
        printf("[EMX] no %s found\n", EMEX_ICON_NAME);
    }

    build_child_path(child_path, path, EMEX_RES_DIR);
    if (dir_exists(child_path)) {
        pkg->has_resources = 1; // optional to have
    }

    printf("[EMX] package '%s' by %s v%s opened successfully\n",
           pkg->info.title, pkg->info.author, pkg->info.version);

    return EMEX_OK;
}

void emex_close_app(emex_package_t *pkg) {
    if (!pkg) return;
    if (pkg->has_icon) {
        bmp_free(&pkg->icon);
        pkg->has_icon = 0;
    }

    printf("[EMX] package closed\n");
}

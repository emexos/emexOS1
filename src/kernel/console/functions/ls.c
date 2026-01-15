#include <kernel/console/console.h>
#include <kernel/file_systems/vfs/vfs.h>

extern char cwd[];

FHDR(cmd_ls) {
    const char *path = s;

    // default to cwd if no path given
    if (!s || *s == '\0') {
        path = cwd;
    }

    // resolve the directory
    fs_node *dir = fs_resolve(path);
    if (!dir) {
        print("error: directory not found\n", GFX_RED);
        print("use: \"ls /boot\" or \"ls /dev\" or any kind of dir.\n", GFX_RED);
        return;
    }

    if (dir->type != FS_DIR) {
        print("error: not a directory\n", GFX_RED);
        return;
    }

    // list all children
    fs_node *child = dir->children;
    if (!child) {
        print("this folder is empty \n", GFX_GRAY_50);
        return;
    }

    int count = 0;
    while (child) {
        // different colors for different types
        u32 color = GFX_WHITE;
        const char *type_str = "";

        if (child->type == FS_DIR) {
            color = GFX_BLUE;
            type_str = "/";
            color = GFX_WHITE;
        } else if (child->type == FS_DEV) {
            color = GFX_WHITE;
            type_str = "*";
            color = GFX_WHITE;
        }

        print(child->name, color);
        print(type_str, color);

        // show size for files
        /*if (child->type == FS_FILE && child->size > 0) {
            char buf[32];
            str_copy(buf, " | ");
            str_append_uint(buf, (u32)child->size);
            str_append(buf, " bytes");
            print(buf, GFX_GRAY_50);
            }*/

        print("  ", GFX_WHITE);
        count++;
        if (count % 6 == 0) {
            print("\n", GFX_WHITE);
        }

        child = child->next;
    }

    if (count % 6 != 0) {
        print("\n", GFX_WHITE);
    }
}

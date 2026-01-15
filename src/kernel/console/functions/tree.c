// src/kernel/console/functions/tree.c
#include <kernel/console/console.h>
#include <kernel/file_systems/vfs/vfs.h>
extern char cwd[];

static void tree_print(fs_node *node, int depth, int is_last)
{
    while (node)
    {
        // indentation
        for (int i = 0; i < depth; i++) {
            print("|   ", GFX_GRAY_50);
        }

        // prefix
        if (is_last && !node->next) {
            print("`-- ", GFX_GRAY_50);
        } else {
            print("|-- ", GFX_GRAY_50);
        }

        // name + color
        u32 color = GFX_WHITE;
        const char *suffix = "";

        if (node->type == FS_DIR) {
            color = GFX_BLUE;
            suffix = "/";
        } else if (node->type == FS_DEV) {
            color = GFX_WHITE;
            suffix = "*";
        }

        print(node->name, color);
        print(suffix, color);
        print("\n", GFX_WHITE);

        // recurse into directories
        if (node->type == FS_DIR && node->children) {
            tree_print(node->children, depth + 1, !node->next);
        }

        node = node->next;
    }
}

FHDR(cmd_tree)
{
    const char *path = s;

    if (!s || *s == '\0') {
        path = cwd;
    }

    fs_node *dir = fs_resolve(path);
    if (!dir) {
        print("error: directory not found\n", GFX_RED);
        return;
    }

    if (dir->type != FS_DIR) {
        print("error: not a directory\n", GFX_RED);
        return;
    }

    // print root
    print(dir->name, GFX_BLUE);
    print("/\n", GFX_BLUE);

    if (!dir->children) {
        print("(empty)\n", GFX_GRAY_50);
        return;
    }

    tree_print(dir->children, 0, 1);
}

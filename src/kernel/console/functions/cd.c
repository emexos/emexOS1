#include <kernel/console/console.h>
#include <kernel/file_systems/vfs/vfs.h>

extern char cwd[];

FHDR(cmd_cd) {
    if (!s || *s == '\0') {
        // cd with no arguments goes to root
        str_copy(cwd, "/");
        return;
    }

    char new_path[MAX_PATH_LEN];

    if (s[0] == '/') {
        str_copy(new_path, s);
    } else {
        str_copy(new_path, cwd);

        if (cwd[str_len(cwd) - 1] != '/') {
            str_append(new_path, "/");
        }

        str_append(new_path, s);
    }

    if (str_equals(s, "..")) {
        int len = str_len(cwd);
        if (len > 1 && cwd[len - 1] == '/') {
            cwd[len - 1] = '\0';
            len--;
        }
        for (int i = len - 1; i >= 0; i--) {
            if (cwd[i] == '/') {
                if (i == 0) {
                    str_copy(cwd, "/");
                } else {
                    cwd[i] = '\0';
                }
                return;
            }
        }
        return;
    }

    // if directory exists
    fs_node *dir = fs_resolve(new_path);
    if (!dir) {
        print("error: directory not found\n", GFX_RED);
        return;
    }

    if (dir->type != FS_DIR) {
        print("error: not a directory\n", GFX_RED);
        return;
    }

    // updates cwd
    str_copy(cwd, new_path);

    // when cwd ends with / (except for the root)
    int len = str_len(cwd);
    if (len > 1 && cwd[len - 1] != '/') {
        str_append(cwd, "/");
    }
}

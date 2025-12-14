#include "vfs.h"
#include <memory/main.h>
#include <string/string.h>
#include <kernel/mem/klime/klime.h>
#include <theme/doccr.h>

int fs_open(const char *path, int flags) {
    fs_node *node = fs_resolve(path);

    // create if O_CREAT and doesn't exist
    if (!node && (flags & O_CREAT)) {
        char ppath[FS_MAX_PATH];
        char fname[64];

        // find last dir slash
        const char *last = path;
        for (const char *p = path; *p; p++) {
            if (*p == '/') last = p;
        }

        int plen = last - path;
        if (plen == 0) plen = 1;

        for (int i = 0; i < plen && i < FS_MAX_PATH - 1; i++) {
            ppath[i] = path[i];
        }
        ppath[plen] = '\0';
        str_copy(fname, last + 1);

        // create file
        fs_node *parent = fs_resolve(ppath);
        if (parent && parent->ops && parent->ops->create) {
            parent->ops->create(parent, fname);
            node = fs_resolve(path);
        }
    }

    if (!node) return -1;

    // alloc file struct
    fs_file *file = (fs_file*)klime_create((klime_t*)fs_klime, sizeof(fs_file));
    if (!file) return -1;

    file->node = node;
    file->pos = 0;
    file->flags = flags;

    // call fs open
    if (node->ops && node->ops->open) {
        if (node->ops->open(node, file) != 0) {
            klime_free((klime_t*)fs_klime, (u64*)file);
            return -1;
        }
    }

    int fd = fs_alloc_fd(file);
    if (fd < 0) {
        klime_free((klime_t*)fs_klime, (u64*)file);
        return -1;
    }

    return fd;
}

int fs_close(int fd) {
    fs_file *file = fs_get_file(fd);
    if (!file) return -1;

    // call fs close
    if (file->node->ops && file->node->ops->close) {
        file->node->ops->close(file);
    }

    klime_free((klime_t*)fs_klime, (u64*)file);
    fs_free_fd(fd);

    return 0;
}

ssize_t fs_read(int fd, void *buf, size_t cnt) {
    fs_file *file = fs_get_file(fd);
    if (!file) return -1;

    if (!file->node->ops || !file->node->ops->read) return -1;

    return file->node->ops->read(file, buf, cnt);
}

ssize_t fs_write(int fd, const void *buf, size_t cnt) {
    fs_file *file = fs_get_file(fd);
    if (!file) return -1;

    if (!file->node->ops || !file->node->ops->write) return -1;

    return file->node->ops->write(file, buf, cnt);
}

int fs_mkdir(const char *path) {
    char ppath[FS_MAX_PATH];
    char dname[64];

    // find last slash
    const char *last = path;
    for (const char *p = path; *p; p++) {
        if (*p == '/') last = p;
    }

    // extract parent path
    int plen = last - path;
    if (plen == 0) plen = 1;

    for (int i = 0; i < plen && i < FS_MAX_PATH - 1; i++) {
        ppath[i] = path[i];
    }
    ppath[plen] = '\0';
    str_copy(dname, last + 1);

    // find parent and create dir
    fs_node *parent = fs_resolve(ppath);
    if (!parent || !parent->ops || !parent->ops->mkdir) return -1;

    int ret = parent->ops->mkdir(parent, dname);

    if (ret == 0) {
        BOOTUP_PRINT("     ", GFX_GRAY_70);
        BOOTUP_PRINT("mkdir ", GFX_ST_WHITE);
        BOOTUP_PRINT(path, GFX_ST_CYAN);
        BOOTUP_PRINT("\n", GFX_ST_WHITE);
    }

    return ret;
}

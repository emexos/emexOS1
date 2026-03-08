#include "../vfs.h"
#include "sysfs.h"
#include <drivers/storage/ata/disk.h>
#include <memory/main.h>
#include <string/string.h>
#include <kernel/mem/klime/klime.h>
#include <theme/doccr.h>


static fs_node* sysfs_lookup(fs_node *dir, const char *name);

static fs_ops sysfs_ops = {
    .open   = NULL,
    .close  = NULL,
    .read   = NULL,
    .write  = NULL,
    .lookup = sysfs_lookup,
    .create = NULL,
    .mkdir  = NULL,
};

static fs_node* sysfs_make_block(void)
{
    fs_node *block = fs_mknode("block", FS_DIR);
    if (!block) return NULL;
    block->ops = &sysfs_ops;

    int count = ATAget_device_count();
    fs_node *tail = NULL;
    for (int i = 0; i < count; i++)
    {
        char devname[4];
        devname[0] = 'h';
        devname[1] = 'd';
        devname[2] = (char)('a' + i);
        devname[3] = '\0';

        fs_node *n = fs_mknode(devname, FS_DIR);
        if (!n) continue;
        n->ops = &sysfs_ops;

        if (!block->children) block->children = n;
        else tail->next = n;
        tail = n;
    }
    return block;
}

static fs_node* sysfs_lookup(fs_node *dir, const char *name)
{
    if (!dir || !name) return NULL;

    // /sys > "block"
    if (str_equals(dir->name, "sys")) {
        if (str_equals(name, "block"))
            return sysfs_make_block();
        return NULL;
    }

    // /sys/block
    if (str_equals(dir->name, "block"))
    {
        int count = ATAget_device_count();
        for (int i = 0; i < count; i++) {
            char devname[4];
            devname[0] = 'h';
            devname[1] = 'd';
            devname[2] = (char)('a' + i);
            devname[3] = '\0';
            if (str_equals(name, devname)) {
                fs_node *n = fs_mknode(devname, FS_DIR);
                if (!n) return NULL;
                n->ops = &sysfs_ops;
                return n;
            }
        }
        return NULL;
    }
    return NULL;
}

static int sysfs_mount(const char *src, const char *tgt, fs_mnt *mnt)
{
    (void)src;
    (void)tgt;

    fs_node *root = fs_mknode("sys", FS_DIR);
    if (!root) return -1;
    root->ops = &sysfs_ops;
    root->children = sysfs_make_block();
    mnt->root = root;

    return 0;
}

static fs_type sysfs_type = {
    .name  = "sysfs",
    .mount = sysfs_mount,
    .ops   = &sysfs_ops,
};

void sysfs_register(void) {
    fs_register(&sysfs_type);
}

void sysfs_refresh(void) {
    extern fs_mnt *fs_get_mount(const char *path);
}
#include "../vfs.h"
#include <memory/main.h>
#include <string/string.h>
#include <kernel/mem/klime/klime.h>
#include <theme/stdclrs.h>
#include <theme/doccr.h>

static fs_node *devfs_root = NULL; // nothings mounted on start

static int devfs_open(fs_node *node, fs_file *file) {
    devfs_data *dev = (devfs_data*)node->priv;
    if (!dev || !dev->mod) return -1;

    // call module open if available
    if (dev->mod->open) {
        dev->handle = dev->mod->open(node->name);
        if (!dev->handle) return -1;
    }

    return 0;
}

static int devfs_close(fs_file *file) {
    devfs_data *dev = (devfs_data*)file->node->priv;
    if (!dev) return -1;

    dev->handle = NULL;
    return 0;
}

static ssize_t devfs_read(fs_file *file, void *buf, size_t cnt) {
    devfs_data *dev = (devfs_data*)file->node->priv;
    if (!dev || !dev->mod || !dev->mod->read) return -1;

    return dev->mod->read(dev->handle, buf, cnt);
}

static ssize_t devfs_write(fs_file *file, const void *buf, size_t cnt) {
    devfs_data *dev = (devfs_data*)file->node->priv;
    if (!dev || !dev->mod || !dev->mod->write) return -1;

    return dev->mod->write(dev->handle, buf, cnt);
}

static fs_node* devfs_lookup(fs_node *dir, const char *name) {
    if (dir->type != FS_DIR) return NULL;

    fs_node *child = dir->children;
    while (child) {
        if (str_equals(child->name, name)) {
            return child;
        }
        child = child->next;
    }

    return NULL;
}

// create a subdir inside a devfs dir (like /dev/input and so on)
static int devfs_mkdir_node(fs_node *dir, const char *name) {
	fs_node *sub = fs_mknode(name, FS_DIR);
    if (!dir || !name) return -1;
    if (devfs_lookup(dir, name)) return 0;
    if (!sub) return -1;

    sub->ops = dir->ops; // inherit devfs ops
    fs_addchild(dir, sub);

    return 0;
}

static fs_ops devfs_ops = {
    .open   = devfs_open,
    .close  = devfs_close,
    .read   = devfs_read,
    .write  = devfs_write,
    .lookup = devfs_lookup,
    .create = NULL,
    .mkdir  = devfs_mkdir_node,
};

static int devfs_mount(const char *src, const char *tgt, fs_mnt *mnt) {
    {
        (void)src;(void)tgt;
    }

    fs_node *root = fs_mknode("dev", FS_DIR);
    if (!root) return -1;

    root->ops = &devfs_ops;
    mnt->root = root;
    devfs_root = root;

    return 0;
}

static fs_type devfs = {
    .name = "devfs",
    .mount = devfs_mount,
    .ops = &devfs_ops,
};

// register devfs type directly
void devfs_register(void) {
    fs_register(&devfs);
}
//fs_register(&devfs);
//cannot be alone standing

// register a device driver module to devfs
// "/dev/input/keyboard0" creates input/ subdir automatically
int devfs_register_device(driver_module *mod)
{
    if (!mod || !devfs_root) return -1;

    // extract device name from mount path
    // "/dev/console" -> "console"
    // "/dev/input/keyboard0" -> "input/keyboard0"
    const char *path = mod->mount;
    const char *name = path;

    if (str_starts_with(path, "/dev/")) {
        name = path + 5; // skip "/dev/"
    }

    // check if there is a subdir
    const char *slash = NULL;
    for (const char *p = name; *p; p++) {
        if (*p == '/') { slash = p; break; }
    }

    fs_node *parent = devfs_root;

    if (slash) {
        // extract subdir name ("input")
        char subdir[64];
        int slen = slash - name;
        if (slen <= 0 || slen >= 64) return -1;

        for (int i = 0; i < slen; i++) subdir[i] = name[i];
        subdir[slen] = '\0';

        // find or create the subdir inside devfs root
        fs_node *sub = devfs_lookup(devfs_root, subdir);
        if (!sub) {
            sub = fs_mknode(subdir, FS_DIR);
            if (!sub) return -1;
            sub->ops = &devfs_ops;
            fs_addchild(devfs_root, sub);
        }

        parent = sub;
        name = slash + 1; // device name after the slash
    }

    // check if already exists in target parent
    if (devfs_lookup(parent, name)) {
        return -1;
    }

    // create device node
    fs_node *node = fs_mknode(name, FS_DEV);
    if (!node) return -1;

    // alloc device data
    devfs_data *data = (devfs_data*)klime_create((klime_t*)fs_klime, sizeof(devfs_data));
    if (!data) {
        klime_free((klime_t*)fs_klime, (u64*)node);
        return -1;
    }

    data->mod = mod;
    data->handle = NULL;

    node->priv = data;
    node->ops  = &devfs_ops;

    fs_addchild(parent, node);

    log("[DEVFS]", "registered ", d);
    BOOTUP_PRINT(mod->mount, white());
    BOOTUP_PRINT("\n", white());

    return 0;
}
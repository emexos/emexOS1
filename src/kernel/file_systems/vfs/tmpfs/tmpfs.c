#include "../vfs.h"
#include <memory/main.h>
#include <string/string.h>
#include <kernel/mem/klime/klime.h>

static int tmpfs_open(fs_node *node, fs_file *file) {
    {
        (void)node;(void)file;
    }
    return 0;
}
static int tmpfs_close(fs_file *file) {(void)file; return 0;}


static ssize_t tmpfs_read(fs_file *file, void *buf, size_t cnt) {
    fs_node *node = file->node;
    if (node->type != FS_FILE) return -1;

    tmpfs_data *data = (tmpfs_data*)node->priv;
    if (!data || !data->data) return 0;

    if (file->pos >= node->size) return 0;

    // calc bytes to read
    size_t to_read = cnt;
    if (file->pos + to_read > node->size) {
        to_read = node->size - file->pos;
    }

    memcpy(buf, (u8*)data->data + file->pos, to_read);
    file->pos += to_read;

    return to_read;
}

static ssize_t tmpfs_write(fs_file *file, const void *buf, size_t cnt) {
    fs_node *node = file->node;
    if (node->type != FS_FILE) return -1;

    tmpfs_data *data = (tmpfs_data*)node->priv;
    if (!data) {
        // alloc data struct
        data = (tmpfs_data*)klime_create((klime_t*)fs_klime, sizeof(tmpfs_data));
        if (!data) return -1;
        data->data = NULL;
        data->cap = 0;
        node->priv = data;
    }

    // expand if needed
    u64 needed = file->pos + cnt;
    if (needed > data->cap) {
        u64 new_cap = (needed + 4095) & ~4095; // round to 4KB

        void *new_data = klime_create((klime_t*)fs_klime, new_cap);
        if (!new_data) return -1;

        // copy old data
        if (data->data && node->size > 0) {
            memcpy(new_data, data->data, node->size);
            klime_free((klime_t*)fs_klime, (u64*)data->data);
        }

        data->data = new_data;
        data->cap = new_cap;
    }

    memcpy((u8*)data->data + file->pos, buf, cnt);
    file->pos += cnt;

    if (file->pos > node->size) {
        node->size = file->pos;
    }

    return cnt;
}

static fs_node* tmpfs_lookup(fs_node *dir, const char *name) {
    if (dir->type != FS_DIR) return NULL;

    // search children
    fs_node *child = dir->children;
    while (child) {
        if (str_equals(child->name, name)) {
            return child;
        }
        child = child->next;
    }

    return NULL;
}

static int tmpfs_create(fs_node *dir, const char *name) {
    if (dir->type != FS_DIR) return -1;
    if (tmpfs_lookup(dir, name)) return -1; // already exists

    fs_node *node = fs_mknode(name, FS_FILE);
    if (!node) return -1;

    node->ops = dir->ops;
    fs_addchild(dir, node);

    return 0;
}

static int tmpfs_mkdir(fs_node *dir, const char *name) {
    if (dir->type != FS_DIR) return -1;
    if (tmpfs_lookup(dir, name)) return -1;

    fs_node *node = fs_mknode(name, FS_DIR);
    if (!node) return -1;

    node->ops = dir->ops;
    fs_addchild(dir, node);

    return 0;
}

static fs_ops tmpfs_ops = {
    .open = tmpfs_open,
    .close = tmpfs_close,
    .read = tmpfs_read,
    .write = tmpfs_write,
    .lookup = tmpfs_lookup,
    .create = tmpfs_create,
    .mkdir = tmpfs_mkdir,

};

static int tmpfs_mount(const char *src, const char *tgt, fs_mnt *mnt) {
    {
        (void)src;(void)tgt;
    }

    fs_node *root = fs_mknode("/", FS_DIR);
    if (!root) return -1;

    root->ops = &tmpfs_ops;
    mnt->root = root;

    return 0;
}

static fs_type tmpfs = {
    .name = "tmpfs",
    .mount = tmpfs_mount,
    .ops = &tmpfs_ops,
};

// register tmpfs type directly
void tmpfs_register(void) {
    fs_register(&tmpfs);
}

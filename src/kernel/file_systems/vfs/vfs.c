#include "vfs.h"
#include <memory/main.h>
#include <string/string.h>
#include <kernel/mem/klime/klime.h>
#include <kernel/graph/theme.h>
#include <theme/doccr.h>

// global state
static fs_mnt mnts[FS_MAX_MNTS];
static fs_file *fds[FS_MAX_FDS];
static fs_type *types[8];
static int type_cnt = 0;

// global klimeptr
void *fs_klime = NULL;

// find mount for path
static fs_mnt* find_mnt(const char *path) {
    fs_mnt *best = NULL;
    size_t best_len = 0;

    for (int i = 0; i < FS_MAX_MNTS; i++) {
        if (!mnts[i].active) continue;

        size_t len = str_len(mnts[i].path);
        if (len > best_len && str_starts_with(path, mnts[i].path)) {
            best = &mnts[i];
            best_len = len;
        }
    }

    return best;
}


//
// fd management
//
int fs_alloc_fd(fs_file *file) {
    // start at 3 for stdin/out/err
    for (int i = 3; i < FS_MAX_FDS; i++) {
        if (!fds[i]) {
            fds[i] = file;
            return i;
        }
    }
    return -1;
}

void fs_free_fd(int fd) {
    if (fd >= 0 && fd < FS_MAX_FDS) {
        fds[fd] = NULL;
    }
}

fs_file* fs_get_file(int fd) {
    if (fd < 0 || fd >= FS_MAX_FDS) return NULL;
    return fds[fd];
}

//
// type registration
//
int fs_register(fs_type *type) {
    if (!type || type_cnt >= 8) return -1;

    types[type_cnt++] = type;

    BOOTUP_PRINT("[FS] ", GFX_GRAY_70);
    BOOTUP_PRINT("registered: ", white());
    BOOTUP_PRINT(type->name, white());
    BOOTUP_PRINT("\n", white());
    return 0;
}

//
// mount
//
int fs_mount(const char *src, const char *tgt, const char *type_name) {
    if (!tgt || !type_name) return -1;

    // find free mount slot
    fs_mnt *mnt = NULL;
    for (int i = 0; i < FS_MAX_MNTS; i++) {
        if (!mnts[i].active) {
            mnt = &mnts[i];
            break;
        }
    }
    if (!mnt) return -1;

    // find filesystem type
    fs_type *type = NULL;
    for (int i = 0; i < type_cnt; i++) {
        if (str_equals(types[i]->name, type_name)) {
            type = types[i];
            break;
        }
    }
    if (!type) return -1;

    // setup mount
    str_copy(mnt->path, tgt);
    mnt->type = type;
    mnt->root = NULL;
    mnt->priv = NULL;

    // call fs mount
    int ret = type->mount(src, tgt, mnt);
    if (ret != 0) return ret;

    mnt->active = 1;

    BOOTUP_PRINT("[FS] ", GFX_GRAY_70);
    BOOTUP_PRINT("mount ", white());
    BOOTUP_PRINT(type_name, white());
    BOOTUP_PRINT(" to ", white());
    BOOTUP_PRINT(tgt, white());
    BOOTUP_PRINT("\n", white());

    return 0;
}

//
// path resolution
//
fs_node* fs_resolve(const char *path) {
    if (!path || path[0] != '/') return NULL;

    // find mount point
    fs_mnt *mnt = find_mnt(path);
    if (!mnt || !mnt->root) {
        return NULL;
    }

    // get relative path
    const char *rel = path + str_len(mnt->path);
    if (*rel == '/') rel++;

    // if empty, return root
    if (*rel == '\0') return mnt->root;

    // traverse path components
    fs_node *cur = mnt->root;
    char comp[64];
    int idx = 0;

    while (*rel) {
        if (*rel == '/') {
            if (idx > 0) {
                comp[idx] = '\0';

                // lookup component in current dir
                if (!cur->ops || !cur->ops->lookup) return NULL;
                cur = cur->ops->lookup(cur, comp);
                if (!cur) return NULL;

                idx = 0;
            }
            rel++;
        } else {
            if (idx < 63) comp[idx++] = *rel;
            rel++;
        }
    }

    // handle last component
    if (idx > 0) {
        comp[idx] = '\0';
        if (!cur->ops || !cur->ops->lookup) return NULL;
        cur = cur->ops->lookup(cur, comp);
    }

    return cur;
}

//
// node helpers
//
fs_node* fs_mknode(const char *name, u8 type) {
    fs_node *node = (fs_node*)klime_create((klime_t*)fs_klime, sizeof(fs_node));
    if (!node) return NULL;

    memset(node, 0, sizeof(fs_node));
    str_copy(node->name, name);
    node->type = type;

    return node;
}

int fs_addchild(fs_node *parent, fs_node *child) {
    if (!parent || !child) return -1;

    child->parent = parent;
    child->next = parent->children;
    parent->children = child;

    return 0;
}


//
// main init
//
void fs_init(void) {
    BOOTUP_PRINT("[FS] ", GFX_GRAY_70);
    BOOTUP_PRINT("init generic VFS\n", white());

    for (int i = 0; i < FS_MAX_MNTS; i++) mnts[i].active = 0;
    for (int i = 0; i < FS_MAX_FDS; i++) fds[i] = NULL;
    for (int i = 0; i < 8; i++) types[i] = NULL;
    type_cnt = 0;
}

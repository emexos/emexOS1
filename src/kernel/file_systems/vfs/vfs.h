#ifndef _VFS_H
#define _VFS_H

#include <types.h>

// file types
#define FS_FILE 0x01
#define FS_DIR  0x02
#define FS_DEV  0x04

// file flags
#define O_RDONLY 0x01
#define O_WRONLY 0x02
#define O_RDWR   0x03
#define O_CREAT  0x04


// seek modes
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

// limits
#define FS_MAX_MNTS 20
#define FS_MAX_FDS  64
#define FS_MAX_PATH 200

typedef struct fs_node fs_node;
typedef struct fs_file fs_file;
typedef struct fs_mnt  fs_mnt;
typedef struct fs_ops  fs_ops;
typedef struct fs_type fs_type;

// node (file/dir/device)
struct fs_node {
    char name[64]; // files
    u8 type;    //#define FS_FILE 0x01
                //#define FS_DIR  0x02
                //#define FS_DEV  0x04

    u64 size;   // file size (bytes)
    u64 inode;

    fs_ops *ops;    // open, write, ...
    void *priv;     // fs-specific data

    fs_node *children; // directorys e.g:
    // tmp/
    // |- log
    // its a children (log) while tmp is the parent
    fs_node *next; // linked lists
    fs_node *parent; // parent path/direcotry
};

// file descriptor
struct fs_file {
    fs_node *node;
    u64 pos;
    u32 flags;
};

// operations
struct fs_ops {
    int (*open)(fs_node *node, fs_file *file);
    int (*close)(fs_file *file);
    ssize_t (*read)(fs_file *file, void *buf, size_t cnt);        // read
    ssize_t (*write)(fs_file *file, const void *buf, size_t cnt); // write
                                                                  // could be used for syscalls

    fs_node* (*lookup)(fs_node *dir, const char *name);
    int (*create)(fs_node *dir, const char *name);
    int (*mkdir)(fs_node *dir, const char *name);

    //tmpfs using ram
    // devfs using modules/modules...
    // ... maybe writing on disk
};

// filesystem type
struct fs_type {
    const char *name;
    int (*mount)(const char *src, const char *tgt, fs_mnt *mnt);
    fs_ops *ops;
};

// mount point
struct fs_mnt {
    char path[FS_MAX_PATH];
    fs_type *type;  // e.g. ptr to tmpfs, devfs
    fs_node *root;
    void *priv;
    int active; // 1 == mounted, 0 == unmounted
};

// main
void fs_init(void);
//void fs_set_klime(void *klime);
void fs_system_init(void *klime);
void fs_register_mods(void);
void fs_create_test_file(void);

int fs_register(fs_type *type);
int fs_mount(const char *src, const char *tgt, const char *type);
fs_node* fs_resolve(const char *path);
fs_node* fs_mknode(const char *name, u8 type);
int fs_addchild(fs_node *parent, fs_node *child);

// fd management
int fs_alloc_fd(fs_file *file);
void fs_free_fd(int fd);
fs_file* fs_get_file(int fd);

// syscall interface
int fs_open(const char *path, int flags);
int fs_close(int fd);
ssize_t fs_read(int fd, void *buf, size_t cnt);
ssize_t fs_write(int fd, const void *buf, size_t cnt);
int fs_mkdir(const char *path);

// global klime pointer (shared by all fs types)
extern void *fs_klime;

// tmpfs
typedef struct {
    void *data;
    u64 cap;
} tmpfs_data;

//void tmpfs_init(void);
void tmpfs_register(void);
//void tmpfs_set_klime(void *klime);

// devfs
#include <kernel/module/module.h>

typedef struct {
    driver_module *mod;
    void *handle;
} devfs_data;

//void devfs_init(void);
void devfs_register(void);
int devfs_register_device(driver_module *mod);

#endif

#ifndef CPIO_H
#define CPIO_H

#include <types.h>

// what the heck is newc format bro..... :(
#define CPIO_MAGIC_NEWC     "070701"
#define CPIO_MAGIC_NEWC_CRC "070702"
#define CPIO_MAGIC_LEN      6

// signals end of the archive
#define CPIO_TRAILER        "TRAILER!!!"

// newc header is always 110 bytes before the filename
#define CPIO_HEADER_SIZE    110

#define CPIO_MODE_IFMT 0170000 // mask to get file type bits
#define CPIO_MODE_IFSOCK 0140000
#define CPIO_MODE_IFLNK 0120000 // link
#define CPIO_MODE_IFREG 0100000 // regular file
#define CPIO_MODE_IFBLK 0060000 // block device (linux-> lsblk)
#define CPIO_MODE_IFDIR 0040000
#define CPIO_MODE_IFCHR 0020000 // character device
#define CPIO_MODE_IFIFO 0010000 // FIFO == named pipe


typedef struct {
    u32 ino;
    u32 mode;
    u32 uid;
    u32 gid;
    u32 nlink;
    u32 mtime;
    u32 filesize;
    u32 devmajor;
    u32 devminor;
    u32 rdevmajor;
    u32 rdevminor;
    u32 namesize;// includes null terminator

    const char *name; // pointer into archive data, NOT a copy
    const u8   *data; // pointer to file data inside archive, NOT a copy
} cpio_entry_t;
typedef struct {
    const u8 *base; // start of CPIO archive in memory
    u64 size; // total archive size in bytes
    u64 offset; // current parse position
} cpio_iter_t;

// retc
#define CPIO_OK         0
#define CPIO_ERR_MAGIC  (-1)   // wrong magic
#define CPIO_ERR_TRUNC  (-2)   // archive trunctaded
#define CPIO_ERR_EOF    (-3)   // TRAILER!!! end


void cpio_iter_init(cpio_iter_t *iter, const u8 *data, u64 size);
int cpio_iter_next(cpio_iter_t *iter, cpio_entry_t *entry);
int cpio_extract_to_vfs(const u8 *data, u64 size, const char *base_path);
int cpio_find(const u8 *data, u64 size, const char *name, cpio_entry_t *entry);


static inline int cpio_is_file(const cpio_entry_t *e) {
    return (e->mode & CPIO_MODE_IFMT) == CPIO_MODE_IFREG;
}
static inline int cpio_is_dir(const cpio_entry_t *e) {
    return (e->mode & CPIO_MODE_IFMT) == CPIO_MODE_IFDIR;
}
static inline int cpio_is_symlink(const cpio_entry_t *e) {
    return (e->mode & CPIO_MODE_IFMT) == CPIO_MODE_IFLNK;
}

#endif

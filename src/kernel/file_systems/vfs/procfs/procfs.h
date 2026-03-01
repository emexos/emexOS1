#ifndef PROCFS_H
#define PROCFS_H

#include <types.h>

// node kinds stored in procfs_data.kind
#define PROCFS_KIND_ROOT   0  // /proc itself
#define PROCFS_KIND_DIR    1  // /proc/<pid>
#define PROCFS_KIND_STATUS 2  // /proc/<pid>/status
#define PROCFS_KIND_MAPS   3  // /proc/<pid>/maps

// stored in node->priv for every procfs node
typedef struct {
    u64 pid;
    u8 kind;
    char *buf;
    u64 len;
} procfs_data;

//void procfs_register(void);

#endif
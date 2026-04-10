#ifndef PROCFS_H
#define PROCFS_H

#include <types.h>
#include <kernel/file_systems/vfs/vfs.h>

// node kinds stored in procfs_data.kind
#define PROCFS_KIND_ROOT   0  // /proc itself
#define PROCFS_KIND_DIR    1  // /proc/<pid>
#define PROCFS_KIND_STATUS 2  // /proc/<pid>/status
#define PROCFS_KIND_MAPS   3  // /proc/<pid>/maps

#define PROCFS_KIND_STAT    4
#define PROCFS_KIND_UPTIME  5
#define PROCFS_KIND_MEMINFO 6
#define PROCFS_KIND_LOADAVG 7
#define PROCFS_KIND_CPUINFO 8

// stored in node->priv for every procfs node
typedef struct {
    u64 pid;
    u8 kind;
    char *buf;
    u64 len;
} procfs_data;

//void procfs_register(void);
fs_node *procfs_dir_node(const char *name, u8 kind);

#endif
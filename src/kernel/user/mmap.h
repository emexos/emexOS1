#ifndef MMAP_ARGS_H
#define MMAP_ARGS_H

typedef struct {
    unsigned long addr;
    unsigned long length;
    int prot;
    int flags;
    int fd;
    long offset;
} mmap_args_t;

// prot flags
#define PROT_NONE  0x00
#define PROT_READ  0x01
#define PROT_WRITE 0x02
#define PROT_EXEC  0x04

// default flags
#define MAP_SHARED  0x01
#define MAP_PRIVATE 0x02
#define MAP_ANONYMOUS 0x20
#define MAP_ANON MAP_ANONYMOUS
#define MAP_FAILED ((void *)-1)

#define _SCAL_MMAP   9
#define _SCAL_MUNMAP 11

#endif
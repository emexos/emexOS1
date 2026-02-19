#ifndef INITRD_H
#define INITRD_H

#define INITRD_MOUNT_POINT "/"
#define INITRD_MODULE_NAME "initrd.cpio"

int initrd_load(void);

#endif

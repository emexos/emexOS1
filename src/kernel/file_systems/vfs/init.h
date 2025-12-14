#pragma once
extern char *logpath;
extern int init_boot_log;

#define ROOTFS "tmpfs"
#define ROOT_MOUNT_DEFAULT "/"
#define TMP_DIRECTORY  "/tmp"

#define DEVFS "devfs"
#define DEV_MOUNT_DEFAULT "/dev"
#define DEV_DIRECTORY  "/dev"
#define _DEV  "/dev/"


#define BOOT_DIRECTORY "/boot"
#define UI_DIRECTORY "/ui"
#define AST_DIRECTORY "/assets"

#define LOGO_DIRECTORY BOOT_DIRECTORY UI_DIRECTORY AST_DIRECTORY
#define LOGO_NAME LOGO_DIRECTORY "/logo.bin"

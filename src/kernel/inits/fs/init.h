#pragma once
extern char *logpath;
extern int init_boot_log;

void initvfs(void);

#define DISK_NAME "hdd0"

#define ROOTFS TMPFS // for now, soon its fat32/ext2
#define ROOT_MOUNT_DEFAULT "/"

#define FAT32 "fat32"
#define FAT32_DIRECTORY  "/"

#define EXT2 "ext2"
#define EXT2_DIRECTORY  "/"

#define TMPFS "tmpfs"
#define TMP_DIRECTORY  "/tmp"


#define DEVFS "devfs"
#define DEV_MOUNT_DEFAULT "/dev"
#define DEV_DIRECTORY  "/dev"
#define _DEV  "/dev/"
#define MOUNT_DEV "/dev/hda"

#define EMX_DIRECTORY "/emr" // == emex system resources
#define _EMX "/emr/" // mount point (/disk)
#define EMCFG_DIRECTORY /*" /emr "*/EMX_DIRECTORY "/config" // == system configs
#define EMAST_DIRECTORY /*" /emr "*/EMX_DIRECTORY "/assets"
#define EMSYS_DIRECTORY /*" /emr "*/EMX_DIRECTORY "/system"
#define EMLOG_DIRECTORY /*" /emr "*/EMSYS_DIRECTORY "/logs"
#define KEYMP_DIRECTORY /*" /emr "*/EMCFG_DIRECTORY "/keymaps"
#define EMDRV_DIRECTORY /*" /emr "*/EMX_DIRECTORY "/drvs"


#define CONF_DIRECTORY /* root/ */ "/.config" // == app configs


#define BOOT_DIRECTORY "/boot"
#define UI_DIRECTORY "/ui"
#define AST_DIRECTORY "/assets"

#define LOGO_DIRECTORY BOOT_DIRECTORY UI_DIRECTORY AST_DIRECTORY
#define LOGO_NAME LOGO_DIRECTORY "/bootlogo.bin"

#define PROCFS "procfs"
#define PROC_MOUNT_DEFAULT "/proc"
#define PROC_DIRECTORY "/proc"


static inline void init_early_boot_log(void) {
    extern int init_boot_log;
    init_boot_log = -1;
}

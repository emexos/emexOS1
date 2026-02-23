#include "init.h"

#include <kernel/file_systems/vfs/vfs.h>

void initvfs(void){
    // only necessary dirs the rest comes from initrd.cpio
    // create standard dirs
    fs_mkdir(DEV_DIRECTORY);
    fs_mkdir(TMP_DIRECTORY);

    fs_mkdir(BOOT_DIRECTORY);
    //emx system requirement paths
    fs_mkdir(EMX_DIRECTORY); // /emr
    fs_mkdir(EMSYS_DIRECTORY);
    fs_mkdir(EMLOG_DIRECTORY);
    //fs_mkdir(EMAST_DIRECTORY);
    //fs_mkdir(EMCFG_DIRECTORY);
    //fs_mkdir(KEYMP_DIRECTORY);
    //fs_mkdir(EMDRV_DIRECTORY);

    //fs_mkdir(CONF_DIRECTORY);
}

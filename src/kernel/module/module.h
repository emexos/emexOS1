#ifndef DRIVER_MODULE_H
#define DRIVER_MODULE_H

#include <types.h>

#define MAX_MODULES 256
#define VERSION_NUM(major, minor, patch, build) \
    ((major << 24) | (minor << 16) | (patch << 8) | build)

typedef struct driver_module_t {
    const char *name;
    const char *mount;      // mount point like /dev/console
    u32 version;        //actually not neccesary but cool

    // init/cleanup
    int (*init)(void);
    void (*fini)(void);

    // not used just for the file system in future
    //
    void *(*open)(const char *path);
    int (*read)(void *handle, void *buf, size_t count);
    int (*write)(void *handle, const void *buf, size_t count);

} driver_module_t;

void module_init(void);
int module_register(driver_module_t *module);
void module_unregister(const char *name);
driver_module_t* module_find(const char *name);
int module_get_count(void);
driver_module_t* module_get_by_index(int idx);

#endif

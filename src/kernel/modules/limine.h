#ifndef LIMINE_MODULE_LOADER_H
#define LIMINE_MODULE_LOADER_H

#include <types.h>

int limine_module_load(const char *source, const char *dest);
int limine_modules_init(void);
int limine_modules_count(void);

// module-info by index
struct limine_file* limine_module_get(int index);

#endif

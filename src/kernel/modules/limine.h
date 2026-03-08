#ifndef LIMINE_MODULE_LOADER_H
#define LIMINE_MODULE_LOADER_H

#include <types.h>

int limine_module_load(const char *source, const char *dest);
void limine_modules_init(void);
int limine_modules_count(void);

// module-info by index
struct limine_file* limine_module_get(int index);

//int limine_module_find_raw(const char *module_name, void **out_addr, u64 *out_size);

#endif
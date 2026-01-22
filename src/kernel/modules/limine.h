#ifndef LIMINE_MODULE_LOADER_H
#define LIMINE_MODULE_LOADER_H

#include <types.h>

// Load a Limine module to VFS path
// source: filename of module (e.g., "logo.bin")
// dest: VFS path where to copy (e.g., "/boot/ui/assets/logo.bin")
int limine_module_load(const char *source, const char *dest);

// Initialize and list all available Limine modules
int limine_modules_init(void);

// Get count of available Limine modules
int limine_modules_count(void);

// Get module info by index
struct limine_file* limine_module_get(int index);

#endif

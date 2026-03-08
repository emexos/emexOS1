#ifndef DEVICE_URANDOM_H
#define DEVICE_URANDOM_H

#include <kernel/module/module.h>

// /dev/urandom
extern driver_module urandom_module;

// shared with /dev/random alias
void *urandom_open_fn(const char *path);
int urandom_read_fn(void *handle, void *buf, size_t count, u64 offset);
int urandom_write_fn(void *handle, const void *buf, size_t count, u64 offset);

#endif
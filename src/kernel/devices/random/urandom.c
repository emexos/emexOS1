#include "urandom.h"

#include <kernel/module/module.h>
#include <kernel/communication/serial.h>
#include <theme/doccr.h>
#include <drivers/drivers.h>
#include <types.h>

#include <kernel/arch/x86_64/rand.h>
/* static u64 xorshift64(void) {
	//https://en.wikipedia.org/wiki/Xorshift
	// https://github.com/jj1bdx/xorshiftplus-c/blob/master/xorshift64star.c
    if (xr_state == 0) {
        xr_state = rdtsc();
        if (xr_state == 0) xr_state = 0xDEADBEEFCAFEBABEULL; // fallback
    }
    u64 x = xr_state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    xr_state = x;
    return x;
} */

static int urandom_init(void) {
    log("[URND]", "init /dev/urandom\n", d);
    return 0;
}

static void urandom_fini(void) {}

// /dev/random will just alias those next 3
void *urandom_open_fn(const char *path) {
    (void)path;
    return (void *)1;
}

int urandom_read_fn(void *handle, void *buf, size_t count, u64 offset) {
    (void)handle;
    (void)offset;
    u8 *out = (u8 *)buf;
    size_t i = 0;

    // fill 8 bytes at a time then handle tail
    while (i + 8 <= count) {
        u64 r = get_hw_rand();
        out[i+0] = (u8)(r);
        out[i+1] = (u8)(r >> 8);
        out[i+2] = (u8)(r >> 16);
        out[i+3] = (u8)(r >> 24);
        out[i+4] = (u8)(r >> 32);
        out[i+5] = (u8)(r >> 40);
        out[i+6] = (u8)(r >> 48);
        out[i+7] = (u8)(r >> 56);
        i += 8;
    }
    if (i < count) {
        u64 r = get_hw_rand();
        while (i < count) {
            out[i++] = (u8)(r & 0xFF);
            r >>= 8;
        }
    }
    return (int)count;
}

int urandom_write_fn(void *handle, const void *buf, size_t count, u64 offset) {
    // accept writes mb for future
    (void)handle;
    (void)buf;
    (void)offset;
    return (int)count;
}

driver_module urandom_module = {
    .name    = URNDNAME,
    .mount   = URNDPATH,
    .version = URNDUNIVERSAL,
    .init    = urandom_init,
    .fini    = urandom_fini,
    .open    = urandom_open_fn,
    .read    = urandom_read_fn,
    .write   = urandom_write_fn,
};
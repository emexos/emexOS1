#pragma once

#include <types.h>

#define TGZ_MAX_BITS 15
#define TGZ_OK 0
#define TGZ_ERR_FORMAT (-1)

typedef struct {
    const u8 *in;
    u8 *out;
    u8 *out_start;
    u32 bit_buf;
    int bit_cnt;
} tgz_stream;

int ungzip(const void *src, void *dst);
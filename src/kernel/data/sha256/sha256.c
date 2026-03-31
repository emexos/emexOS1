#include "sha256.h"
#include <types.h>
#include <memory/main.h>
#include <string/string.h>

/*
 * sha256
 * made by @msaid5860
 * ported to emexOS by @emex
 */

#define SHA2_SHFR(x, n)((x) >> (n))
#define SHA2_ROTR(x, n)(((x) >> (n)) | ((x) << (32 - (n))))
#define SHA2_CH(x, y, z)(((x) & (y)) ^ (~(x) & (z)))
#define SHA2_MAJ(x, y, z)(((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

#define SHA2_F1(x) (SHA2_ROTR(x, 2) ^  SHA2_ROTR(x, 13) ^ SHA2_ROTR(x, 22))
#define SHA2_F2(x) (SHA2_ROTR(x, 6) ^  SHA2_ROTR(x, 11) ^ SHA2_ROTR(x, 25))
#define SHA2_F3(x) (SHA2_ROTR(x, 7) ^  SHA2_ROTR(x, 18) ^ SHA2_SHFR(x, 3 ))
#define SHA2_F4(x) (SHA2_ROTR(x, 17) ^ SHA2_ROTR(x, 19) ^ SHA2_SHFR(x, 10))

static const u32 K[64] = {
    0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u,
    0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
    0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u,
    0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
    0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu,
    0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
    0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u,
    0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
    0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u,
    0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
    0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u,
    0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
    0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u,
    0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
    0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u,
    0x90beffeau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u,
};

static char sha256_buffers[4][65];
static int sha256_buf_idx = 0;

static const char hex_map[] = "0123456789abcdef";

const char *sha256_hash(const char *src) {
    if (!src) return 0;

    size_t len = (size_t)str_len(src);
    size_t offset = 0;

    u32 h[8] = {
        0x6a09e667u, 0xbb67ae85u, 0x3c6ef372u, 0xa54ff53au,
        0x510e527fu, 0x9b05688cu, 0x1f83d9abu, 0x5be0cd19u,
    };
    u64 bitlen = (u64)len * 8;
    u8 chunk[64];

    int end = 0;
    int extra = 0;

    while (!end) {
        memset(chunk, 0, 64);

        size_t rem = len - offset;
        if (rem >= 64)
        {
            memcpy(chunk, (const u8 *)src + offset, 64);
            offset += 64;
        } else {
            if (!extra) {
                if (rem > 0) memcpy(chunk, (const u8 *)src + offset, rem);

                chunk[rem] = 0x80;

                if (rem < 56) {
                    // length fits in this block
                    for (int i = 0; i < 8; i++) chunk[63 - i] = (u8)(bitlen >> (i * 8));
                    end = 1;
                } else {
                    // need one more block for the length
                    extra = 1;
                }
                offset = len;
            } else {
                for (int i = 0; i < 8; i++) chunk[63 - i] = (u8)(bitlen >> (i * 8));
                end = 1;
            }
        }

        // expand
        // 16 words >> 64 words
        u32 w[64];
        for (int i = 0; i < 16; i++)
        {
            w[i] = ((u32)chunk[i * 4]     << 24)
                 | ((u32)chunk[i * 4 + 1] << 16)
                 | ((u32)chunk[i * 4 + 2] << 8)
                 | ((u32)chunk[i * 4 + 3]);
        }
        for (int i = 16; i < 64; i++) {
            w[i] = SHA2_F4(w[i - 2]) +  w[i - 7]
                 + SHA2_F3(w[i - 15]) + w[i - 16];
        }

        // compression
        u32 v[8];
        memcpy(v, h, sizeof(h));

        for (int i = 0; i < 64; i++) {
            u32 t1 = v[7] + SHA2_F2(v[4]) + SHA2_CH(v[4], v[5], v[6]) + K[i] + w[i];
            u32 t2 = SHA2_F1(v[0]) + SHA2_MAJ(v[0], v[1], v[2]);
            v[7] = v[6];
            v[6] = v[5];
            v[5] = v[4];
            v[4] = v[3]+t1;
            v[3] = v[2];
            v[2] = v[1];
            v[1] = v[0];
            v[0] = t1+t2;
        }

        for (int i = 0; i < 8; i++) h[i] += v[i];
    }

    // convert digest to hex string
    char *dst = sha256_buffers[sha256_buf_idx];
    sha256_buf_idx = (sha256_buf_idx + 1) & 3; // mod 4

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 4; j++) {
            u8 byte = (u8)(h[i] >> ((3 - j) * 8));

            dst[(i * 8) + (j * 2)]= hex_map[(byte >> 4) & 0x0F];
            dst[(i * 8) + (j * 2) + 1] = hex_map[byte & 0x0F ];
        }
    }
    dst[64] = '\0';

    return dst;
}
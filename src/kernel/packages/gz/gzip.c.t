#include <freestanding/stdint.h>
#include <freestanding/stddef.h>
#include <main/string.h>
#include <main/gzip.h>

//
// MADE BY @msaid5860
// gzip parser
// (currently not used)
//

static uint32_t tgz_get_bits(tgz_stream *s, int bits) {
    while (s->bit_cnt < bits) {
        s->bit_buf |= ((uint32_t)(*s->in++)) << s->bit_cnt;
        s->bit_cnt += 8;
    }
    uint32_t val = s->bit_buf & ((1 << bits) - 1);
    s->bit_buf >>= bits;
    s->bit_cnt -= bits;
    return val;
}

// --- Huffman Logic ---
typedef struct {
    uint16_t counts[TGZ_MAX_BITS + 1];
    uint16_t symbols[TGZ_MAX_BITS + 1][288];
} tgz_huffman;

static void tgz_build_tree(tgz_huffman *tree, const uint8_t *lens, int n) {
    int i, codes[TGZ_MAX_BITS + 1];
    memset(tree->counts, 0, sizeof(tree->counts));
    for (i = 0; i < n; i++) tree->counts[lens[i]]++;
    tree->counts[0] = 0;

    int code = 0;
    for (i = 1; i <= TGZ_MAX_BITS; i++) {
        code = (code + tree->counts[i-1]) << 1;
        codes[i] = code;
    }

    int offsets_map[TGZ_MAX_BITS + 1];
    for(i=0; i<=TGZ_MAX_BITS; i++) offsets_map[i] = 0;

    for (i = 0; i < n; i++) {
        if (lens[i] != 0) tree->symbols[lens[i]][offsets_map[lens[i]]++] = (uint16_t)i;
    }
}

static int tgz_decode_symbol(tgz_stream *s, tgz_huffman *tree) {
    int len, code = 0, first = 0, count;
    for (len = 1; len <= TGZ_MAX_BITS; len++) {
        code |= tgz_get_bits(s, 1);
        count = tree->counts[len];
        if (code - count < first) return tree->symbols[len][code - first];
        first += count;
        first <<= 1;
        code <<= 1;
    }
    return -1;
}

static const uint8_t CLCL_ORDER[19] = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };

// --- Inflate ---
static int tgz_inflate(tgz_stream *s) {
    int final = 0;
    while (!final) {
        final = tgz_get_bits(s, 1);
        int type = tgz_get_bits(s, 2);

        if (type == 0) { // Uncompressed
            s->bit_buf >>= (s->bit_cnt & 7);
            s->bit_cnt &= ~7;
            uint16_t len = (uint16_t)tgz_get_bits(s, 16);
            uint16_t nlen = (uint16_t)tgz_get_bits(s, 16);
            if (len != (uint16_t)~nlen) return TGZ_ERR_FORMAT;

            // Note: No output bounds check here!
            memcpy(s->out, s->in, len);
            s->in += len;
            s->out += len;

        } else if (type == 1 || type == 2) {
            tgz_huffman lit_tree, dist_tree;
            if (type == 1) { // Fixed
                uint8_t lens[288], dist_lens[32];
                int i;
                for (i=0; i<144; i++) lens[i] = 8;
                for (; i<256; i++) lens[i] = 9;
                for (; i<280; i++) lens[i] = 7;
                for (; i<288; i++) lens[i] = 8;
                tgz_build_tree(&lit_tree, lens, 288);
                for (i=0; i<32; i++) dist_lens[i] = 5;
                tgz_build_tree(&dist_tree, dist_lens, 32);
            } else { // Dynamic
                int hlit = tgz_get_bits(s, 5) + 257;
                int hdist = tgz_get_bits(s, 5) + 1;
                int hclen = tgz_get_bits(s, 4) + 4;
                uint8_t code_lens[19];
                memset(code_lens, 0, 19);
                for (int i = 0; i < hclen; i++) code_lens[CLCL_ORDER[i]] = (uint8_t)tgz_get_bits(s, 3);

                tgz_huffman code_tree;
                tgz_build_tree(&code_tree, code_lens, 19);
                uint8_t lens[288 + 32];
                int n = 0;
                while (n < hlit + hdist) {
                    int sym = tgz_decode_symbol(s, &code_tree);
                    if (sym < 16) lens[n++] = (uint8_t)sym;
                    else if (sym == 16) {
                        int copy_len = tgz_get_bits(s, 2) + 3;
                        uint8_t prev = lens[n-1];
                        while (copy_len--) lens[n++] = prev;
                    } else if (sym == 17) {
                        int copy_len = tgz_get_bits(s, 3) + 3;
                        while (copy_len--) lens[n++] = 0;
                    } else if (sym == 18) {
                        int copy_len = tgz_get_bits(s, 7) + 11;
                        while (copy_len--) lens[n++] = 0;
                    }
                }
                tgz_build_tree(&lit_tree, lens, hlit);
                tgz_build_tree(&dist_tree, lens + hlit, hdist);
            }

            while (1) {
                int sym = tgz_decode_symbol(s, &lit_tree);
                if (sym < 256) {
                    *s->out++ = (uint8_t)sym;
                } else if (sym == 256) {
                    break;
                } else {
                    sym -= 257;
                    static const int lbase[] = {3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,43,51,59,67,83,99,115,131,163,195,227,258};
                    static const int lext[] = {0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0};
                    int len = lbase[sym] + tgz_get_bits(s, lext[sym]);

                    int dist_sym = tgz_decode_symbol(s, &dist_tree);
                    static const int dbase[] = {1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577};
                    static const int dext[] = {0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};
                    int dist = dbase[dist_sym] + tgz_get_bits(s, dext[dist_sym]);

                    // LZ77 Byte Copy
                    uint8_t *src_copy = s->out - dist;
                    for (int i = 0; i < len; i++) *s->out++ = *src_copy++;
                }
            }
        } else return TGZ_ERR_FORMAT;
    }
    return TGZ_OK;
}

// --- Public Wrapper ---

/*
 * ungzip
 * * src: pointer to GZIP data
 * * dst: pointer to destination buffer (MUST BE LARGE ENOUGH)
 * * Returns: Number of bytes written to dst
 */
int ungzip(const void *src, void *dst) {
    const uint8_t *in = (const uint8_t *)src;

    // Check Header (0x1F, 0x8B, 0x08)
    if (in[0] != 0x1F || in[1] != 0x8B || in[2] != 8) return TGZ_ERR_FORMAT;

    uint8_t flags = in[3];
    const uint8_t *data_start = in + 10;

    // Skip Header Fields
    if (flags & 0x04) { // FEXTRA
        uint16_t extra_len = data_start[0] | (data_start[1] << 8);
        data_start += 2 + extra_len;
    }
    if (flags & 0x08) { // FNAME
        while (*data_start) data_start++;
        data_start++;
    }
    if (flags & 0x10) { // FCOMMENT
        while (*data_start) data_start++;
        data_start++;
    }
    if (flags & 0x02) data_start += 2; // FHCRC

    // Initialize Stream
    tgz_stream stream;
    stream.in = data_start;
    stream.out = (uint8_t *)dst;
    stream.out_start = (uint8_t *)dst;
    stream.bit_buf = 0;
    stream.bit_cnt = 0;

    // Decompress until End-of-Block symbol found
    if (tgz_inflate(&stream) != TGZ_OK) return TGZ_ERR_FORMAT;

    return (int)(stream.out - stream.out_start);
}

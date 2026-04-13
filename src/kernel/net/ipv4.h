#ifndef NET_IPV4_H
#define NET_IPV4_H

// https://wiki.osdev.org/IPv4

#include <types.h>
#include "eth.h"

#define IPV4_PROTO_ICMP  	1 /*make ping */
#define IPV4_PROTO_TCP   	6
#define IPV4_PROTO_UDP   17

#define IPV4_TTL_DEFAULT 64
#define IPV4_HDR_LEN 20

typedef struct
{
    u8  	version_ihl;
    u8  	protocol;
    u8  	src[4];
    u8  	dst[4];
} __attribute__((packed)) ipv4_hdr_t;


#endif
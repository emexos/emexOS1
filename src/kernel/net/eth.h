#ifndef NET_ETH_H
#define NET_ETH_H

// https://wiki.osdev.org/Ethernet

#include <types.h>

#define ETH_ALEN     6
#define ETH_HDR_LEN  14
#define ETH_MAX_LEN  1500

#define ETH_TYPE_ARP  0x0806
#define ETH_TYPE_IPV4 0x0800

typedef struct {
    u8     dst[ETH_ALEN];
    u8	   src[ETH_ALEN];
    u16    type;
} __attribute__((packed)) eth_frame_t;


void eth_init(void);
int eth_send(const u8 dst[ETH_ALEN], u16 type, const void *payload, u16 len);
void eth_recv(const void *frame, u16 len);
void eth_get_mac(u8 out[ETH_ALEN]);

#endif
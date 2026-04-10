// https://wiki.osdev.org/Communications#Ethernet
#include "eth.h"
#include "arp.h"
#include <drivers/net/net.h>
#include <memory/main.h>
#include <kernel/communication/serial.h>
#include <theme/doccr.h>

static u8 g_mac[ETH_ALEN];

static u16 htons16(u16 v)
{
    return (u16)((v << 8) | (v >> 8));
}

void eth_init(void)
{
    netdrivers_get_mac(g_mac);
    log("[ETH]", "ready\n", d);
}

void eth_get_mac(u8 out[ETH_ALEN])
{
    for (int i = 0; i < ETH_ALEN; i++) out[i] = g_mac[i];
}

int eth_send(const u8 dst[ETH_ALEN], u16 type, const void *payload, u16 len)
{
    if (!payload || len == 0 || len > ETH_MAX_LEN) return -1;

    /* stack buffer; max ETH_MAX_LEN + header */
    static u8 buf[ETH_HDR_LEN + ETH_MAX_LEN];

    eth_frame_t *f = (eth_frame_t *)buf;

    for (int i = 0; i < ETH_ALEN; i++) f->dst[i] = dst[i];
    for (int i = 0; i < ETH_ALEN; i++) f->src[i] = g_mac[i];
    f->type = htons16(type);

    memcpy(buf + ETH_HDR_LEN, payload, len);

    return net_send(buf, (u16)(ETH_HDR_LEN + len));
}

void eth_recv(const void *raw, u16 total_len)
{
    if (!raw || total_len < ETH_HDR_LEN) return;

    const eth_frame_t *f = (const eth_frame_t *)raw;
    u16 type = htons16(f->type);
    const void *payload = (const u8 *)raw + ETH_HDR_LEN;
    u16 plen = (u16)(total_len - ETH_HDR_LEN);

    switch (type)
    {
        case ETH_TYPE_ARP:
            arp_recv(payload, plen);
            break;

        /* ipv4:*/



        // empty




        case ETH_TYPE_IPV4:
            break;

        default:
            break;
    }
}
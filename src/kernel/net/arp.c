// https://wiki.osdev.org/Address_Resolution_Protocol
#include "arp.h"
#include "eth.h"
#include <kernel/communication/serial.h>
#include <memory/main.h>
#include <theme/doccr.h>

static arp_entry_t g_table[ARP_TABLE_SIZE];
static u8 g_ip[4];

static const u8 broadcast[ETH_ALEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

static u16 htons16(u16 v)
{
    return (u16)((v << 8) | (v >> 8));
}

static int ip_eq(const u8 a[4], const u8 b[4])
{
    return a[0]==b[0] && a[1]==b[1] && a[2]==b[2] && a[3]==b[3];
}

/* add or update an entry in the arp table */
static void arp_table_set(const u8 ip[4], const u8 mac[ETH_ALEN])
{
    /* update existing */
    for (int i = 0; i < ARP_TABLE_SIZE; i++)
    {
        if (g_table[i].valid && ip_eq(g_table[i].ip, ip))
        {
            for (int j = 0; j < ETH_ALEN; j++) g_table[i].mac[j] = mac[j];
            return;
        }
    }

    /* find free slot */
    for (int i = 0; i < ARP_TABLE_SIZE; i++)
    {
        if (!g_table[i].valid)
        {
            for (int j = 0; j < 4;j++) g_table[i].ip[j]  = ip[j];
            for (int j = 0; j < ETH_ALEN; j++) g_table[i].mac[j] = mac[j];
            g_table[i].valid = 1;
            return;
        }
    }

    /* table full ->overwrite slot 0 */
    for (int j = 0; j < 4;        j++) g_table[0].ip[j]  = ip[j];
    for (int j = 0; j < ETH_ALEN; j++) g_table[0].mac[j] = mac[j];
}

void arp_init(void)
{
    memset(g_table, 0, sizeof(g_table));
    log("[ARP]", "table ready.\n", d);
}

int arp_lookup(const u8 ip[4], u8 mac_out[ETH_ALEN])
{
    for (int i = 0; i < ARP_TABLE_SIZE; i++)
    {
        if (g_table[i].valid && ip_eq(g_table[i].ip, ip))
        {
            for (int j = 0; j < ETH_ALEN; j++) mac_out[j] = g_table[i].mac[j];
            return 0;
        }
    }
    return -1; /* not found */
}

int arp_request(const u8 target_ip[4])
{
    arp_packet_t pkt;

    pkt.hw_type   		= htons16(1);
    pkt.proto     		= htons16(0x0800);
    pkt.hw_len    		= ETH_ALEN;
    pkt.proto_len 		= 4;
    pkt.op        		= htons16(ARP_OP_REQUEST);

    eth_get_mac(pkt.sender_mac);

    for (int i = 0; i < 4;i++) pkt.sender_ip[i]  	= g_ip[i];
    for (int i = 0; i < ETH_ALEN; i++) pkt.target_mac[i] = 0x00;
    for (int i = 0; i < 4; i++) pkt.target_ip[i]  = target_ip[i];

    return eth_send(broadcast, ETH_TYPE_ARP, &pkt, sizeof(pkt));
}

void arp_recv(const void *raw, u16 len)
{
    if (!raw || len < (u16)sizeof(arp_packet_t)) return;

    const arp_packet_t *pkt = (const arp_packet_t *)raw;

    if (htons16(pkt->hw_type)   != 1)      return;
    if (htons16(pkt->proto)     != 0x0800) return;
    if (pkt->hw_len    			!= ETH_ALEN)   return;
    if (pkt->proto_len 			!= 4)          return;

    u16 op = htons16(pkt->op);

    /* always learn the sender */
    arp_table_set(pkt->sender_ip, pkt->sender_mac);

    if (op == ARP_OP_REQUEST && ip_eq(pkt->target_ip, g_ip))
    {
        /* send reply*/
        arp_packet_t reply;

        reply.hw_type   	= htons16(1);
        reply.proto     	= htons16(0x0800);
        reply.hw_len    	= ETH_ALEN;
        reply.proto_len 	= 4;
        reply.op        	= htons16(ARP_OP_REPLY);

        eth_get_mac(reply.sender_mac);
        for (int i = 0; i < 4; i++) reply.sender_ip[i]  = g_ip[i];
        for (int i = 0; i < ETH_ALEN; i++) reply.target_mac[i] = pkt->sender_mac[i];
        for (int i = 0; i < 4;i++) reply.target_ip[i]  		= pkt->sender_ip[i];

        eth_send(pkt->sender_mac, ETH_TYPE_ARP, &reply, sizeof(reply));
    }
}
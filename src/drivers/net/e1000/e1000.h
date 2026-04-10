#ifndef E1000_H
#define E1000_H


#include <types.h>

#include "../layers.h"

#define E1000_VENDOR    0x8086
#define E1000_82540EM  	0x100E  // qemu default
#define E1000_82545EM  	0x100F
#define E1000_82574L   	0x10D3

// mmio offsets
#define E1000_CTRL   	0x00000
#define E1000_EERD   	0x00014
#define E1000_IMC    	0x000D8
#define E1000_RCTL   	0x00100
#define E1000_TCTL   	0x00400
#define E1000_TIPG   	0x00410
#define E1000_RDBAL  	0x02800
#define E1000_RDBAH  	0x02804
#define E1000_RDLEN  	0x02808
#define E1000_RDH    	0x02810
#define E1000_RDT    	0x02818
#define E1000_TDBAL  	0x03800
#define E1000_TDBAH  	0x03804
#define E1000_TDLEN  	0x03808
#define E1000_TDH    	0x03810
#define E1000_TDT    	0x03818
#define E1000_MTA    	0x05200
#define E1000_RAL    	0x05400
#define E1000_RAH    	0x05404

#define E1000_CTRL_SLU   	(1u << 6)
#define E1000_CTRL_ASDE  	(1u << 5)
#define E1000_CTRL_RST   	(1u << 26)

#define E1000_RCTL_EN    	(1u << 1)
#define E1000_RCTL_BAM   	(1u << 15)
#define E1000_RCTL_SECRC 	(1u << 26)

#define E1000_TCTL_EN    	(1u << 1)
#define E1000_TCTL_PSP   	(1u << 3)

#define E1000_RXD_DD  	0x01
#define E1000_TXD_DD  	0x01
#define E1000_TXD_EOP 	0x01
#define E1000_TXD_FCS 	0x02
#define E1000_TXD_RS  	0x08

#define E1000_RX_COUNT 32
#define E1000_TX_COUNT 32
#define E1000_BUF_SIZE 2048

typedef struct
{
    u64 	addr;
    u16 	length;
    u16 	checksum;
    u8  	status;
    u8  	errors;
    u16 	special;
} __attribute__((packed)) e1000_rx_desc_t;
typedef struct
{
    u64		addr;
    u16 	length;
    u8  	cso;
    u8  	cmd;
    u8  	status;
    u8  	css;
    u16 	special;
} __attribute__((packed)) e1000_tx_desc_t;

int e1000_init(void);
int e1000_send(const void *data, u16 len);
int e1000_recv(void *buf, u16 max_len);
int e1000_present(void);
void e1000_get_mac(u8 mac[6]);


#endif
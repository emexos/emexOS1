#ifndef RTL8139_H
#define RTL8139_H

/* network driver layer */
#include "../layers.h"

#include <types.h>

int rtl8139_init(void);
int rtl8139_send(const void *data, u16 len);
int rtl8139_recv(void *buf, u16 max_len);
void rtl8139_get_mac(u8 mac[6]);

#endif
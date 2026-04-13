#include "net.h"
#include "eth.h"
#include "arp.h"
#include "ipv4.h"

#include <kernel/communication/serial.h>
#include <theme/doccr.h>

#include <drivers/net/net.h>

int net_is_available(void)
{
	return netdrivers_available(); /* return 0 on failure */
}

/* qemu default address ig */
static const u8 g_default_ip[4] = { 10, 0, 2, 15 };

void netstack_init(void)
{
    eth_init();
    arp_init();
    //while (1)
    //{
    //    net_poll();
    //}
}
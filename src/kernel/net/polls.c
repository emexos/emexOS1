//#include <drivers/net/e1000/e1000.h>
#include <kernel/communication/serial.h>
#include "net.h"
#include "eth.h"
#include <drivers/net/net.h>
#include <string/string.h>
#include <kernel/kernel_processes/bootscreen/print.h>
#include <kernel/kernel_processes/bootscreen/log.h>

void net_poll(void)
{
	void* data;

	//while ((len = e1000_recv(&data)))
	//{
	//	printf("packet len=%d\n", len);
	//}
 	static u8 buf[1518];
    int len;

    while ((len = netdrivers_recv(buf, sizeof(buf))) > 0)
    {
        logf(d, "NET", "packet received, len=%d", len);

        eth_recv(buf, (u16)len);
    }
}
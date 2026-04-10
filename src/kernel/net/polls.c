//#include <drivers/net/e1000/e1000.h>
#include <kernel/communication/serial.h>

void net_poll() {
	void* data;
	int len;

	//while ((len = e1000_recv(&data)))
	//{
	//	printf("packet len=%d\n", len);
	//}
}
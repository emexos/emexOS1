#ifndef NETLAYER
#define NETLAYER


#include <types.h>

typedef struct
{
    int  	(*init) (void); /* initialize network drivers */
    int  	(*send) (const void *data, u16 len); /* send data */
    int  	(*recv) (void *buf, u16 max_len); /* recive data */
    void 	(*get_mac)(u8 mac[6]);
} netdrivers_driver_t;

int netdrivers_init(void);
int netdrivers_send(const void *data, u16 len);
int netdrivers_recv(void *buf, u16 max_len);

void netdrivers_get_mac(u8 mac[6]);

#endif
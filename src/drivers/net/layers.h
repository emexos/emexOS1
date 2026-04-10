#ifndef NETLAYER
#define NETLAYER


#include <types.h>

typedef struct
{
    int  	(*init) (void); /* initialize network drivers */
    int  	(*send) (const void *data, u16 len); /* send data */
    int  	(*recv) (void *buf, u16 max_len); /* recive data */
    void 	(*get_mac)(u8 mac[6]);
} net_driver_t;

int net_init(void);
int net_send(const void *data, u16 len);
int net_recv(void *buf, u16 max_len);

void net_get_mac(u8 mac[6]);

#endif
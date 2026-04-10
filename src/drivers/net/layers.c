#include "layers.h"
#include "e1000/e1000.h"
#include "rtl8139/rtl8139.h"

static net_driver_t *active = 0;

/* net driver instances */
static net_driver_t e1000_driver =
{
    .init    =  e1000_init,
    .send    =  e1000_send,
    .recv    =  e1000_recv,
    .get_mac =  e1000_get_mac
};

static net_driver_t rtl8139_driver =
{
    .init    =  rtl8139_init,
    .send    =  rtl8139_send,
    .recv    =  rtl8139_recv,
    .get_mac =  rtl8139_get_mac
};

//

/*
 * initializes network layer
 *  selects the working network sytem
 */
int net_init(void)
{
    /* Intel E1000 first */
    if (e1000_driver.init() == 0) {
        active = &e1000_driver;
        return 0;
    }

    /* fallback to Realtek RTL8139 */
    if (rtl8139_driver.init() == 0) {
        active = &rtl8139_driver;
        return 0;
    }

    // others...

    /* no supported NIC found */
    return -1;
}

/*
 * send packetes
 */
int net_send(const void *data, u16 len)
{
    if (!active)
        return -1;

    return active->send(data, len);
}
/*
 * receive packetes
 */
int net_recv(void *buf, u16 max_len)
{
    if (!active)
        return 0;

    return active->recv(buf, max_len);
}

void net_get_mac(u8 mac[6])
{
    if (!active)
        return;

    active->get_mac(mac);
}

/* availability check
 * if no driver is used, return 0
 */
int net_available(void)
{
    return active != 0;
}
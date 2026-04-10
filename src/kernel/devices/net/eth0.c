#include "eth0.h"
#include <kernel/module/module.h>
#include <kernel/communication/serial.h>
#include <theme/doccr.h>
#include <drivers/drivers.h>
#include <drivers/net/net.h>

/*
 * eth0 device in /dev/net/eth0
 * sets up a network layer and selects a working nic driver
 */
static int eth0_init(void)
{
    log("[ETH0]", "init /dev/net/eth0\n", d);
    return net_init();
}

/* Returns a dummy handle if a network driver is available. */
static void *eth0_open(const char *path)
{
    (void)path;

    /*
     * if no driver was initialized deny access
     *
     * net_send() will auto fail if no active driver exists so we use that for checking availability
     */
    if (!net_available())
        return NULL; /* failure */

    return (void *)1;
}

/*receives a packet into the buffer */
static int eth0_read(void *handle, void *buf, size_t count, u64 offset)
{
    (void)handle;
    (void)offset;

    if (!buf || !count)
        return 0;

    u16 max = count > 0xFFFF ? 0xFFFF : (u16)count; /* driver interface is u16,
    which is the limit */

    return net_recv(buf, max);
}

/* send a packet through a active network driver*/
static int eth0_write(void *handle, const void *buf, size_t count, u64 offset)
{
    (void)handle;
    (void)offset;

    if (!buf || !count)
        return -1;

    u16 len = count > 0xFFFF ? 0xFFFF : (u16)count; /* driver interface is u16,
    which is the limit */

    int ret = net_send(buf, len);

    return (ret == 0) ? (int)count : -1;
}


static void eth0_fini(void){}


driver_module eth0_module =
{
    .name    = ETH0NAME,
    .mount   = ETH0PATH,
    .version = ETH0UNIVERSAL,
    .init    = eth0_init,
    .fini    = eth0_fini,
    .open    = eth0_open,
    .read    = eth0_read,
    .write   = eth0_write,
};
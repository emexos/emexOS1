#include "tty1.h"
#include "tty.h"

#include <kernel/module/module.h>
#include <kernel/communication/serial.h>
#include <theme/doccr.h>
#include <drivers/drivers.h>
#include <types.h>

void tty1_write_char(char c) { tty_write_char(1, c); }
void tty1_set_echo_mode(int mode){ tty_set_echo(1, mode); }
int tty1_get_echo_mode(void){ return tty_get_echo(1); }

static int tty1_init(void)
{
	tty_init();
    log("[TTY1]", "init /dev/tty1\n", d);
    return 0;
}

static void tty1_fini(void) {}

static void *tty1_open(const char *path)
{
    (void)path;
    return (void *)1;
}

static int tty1_dev_write(void *handle, const void *buf, size_t count, u64 offset)
{
    (void)handle; (void)offset;
    const char *p = (const char *)buf;
    for (size_t i = 0; i < count; i++) tty_write_char(1, p[i]);
    return (int)count;
}

static int tty1_dev_read(void *handle, void *buf, size_t count, u64 offset)
{
    (void)handle; (void)offset;
    return tty_dev_read(1, buf, count);
}

driver_module tty1_module =
{
    .name    = TTY1NAME,
    .mount   = TTY1PATH,
    .version = TTY1UNIVERSAL,
    .init    = tty1_init,
    .fini    = tty1_fini,
    .open    = tty1_open,
    .read    = tty1_dev_read,
    .write   = tty1_dev_write,
};
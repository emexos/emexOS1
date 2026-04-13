#include "tty0.h"
#include "tty.h"

#include <kernel/module/module.h>
#include <kernel/communication/serial.h>
#include <string/string.h>
#include <kernel/arch/x86_64/exceptions/irq.h>
#include <drivers/ps2/keyboard/keyboard.h>
#include <theme/doccr.h>
#include <drivers/drivers.h>
#include <types.h>
#include <kernel/graph/graphics.h>
#include <kernel/kernel_processes/fm/fm.h>
#include <kernel/graph/theme.h>

void tty0_write_char(char c) { tty_write_char(0, c); }
void tty0_set_echo_mode(int mode){ tty_set_echo(0, mode); }
int tty0_get_echo_mode(void){ return tty_get_echo(0); }

static int tty0_init(void)
{
    log("[TTY0]", "init /dev/tty0\n", d);
    return 0;
}

static void tty0_fini(void) {}

static void *tty0_open(const char *path)
{
    (void)path;
    return (void *)1;
}

static int tty0_dev_write(void *handle, const void *buf, size_t count, u64 offset)
{
    (void)handle; (void)offset;
    const char *p = (const char *)buf;
    for (size_t i = 0; i < count; i++) tty_write_char(0, p[i]);
    return (int)count;
}

static int tty0_dev_read(void *handle, void *buf, size_t count, u64 offset)
{
    (void)handle; (void)offset;
    return tty_dev_read(0, buf, count);
}

driver_module tty0_module =
{
    .name    = TTY0NAME,
    .mount   = TTY0PATH,
    .version = TTY0UNIVERSAL,
    .init    = tty0_init,
    .fini    = tty0_fini,
    .open    = tty0_open,
    .read    = tty0_dev_read,
    .write   = tty0_dev_write,
};
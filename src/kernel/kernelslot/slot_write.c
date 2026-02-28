#include "slot.h"
#include <kernel/file_systems/fat32/fat32.h>
#include <string/string.h>
#include <kernel/communication/serial.h>
#include <theme/doccr.h>

#define LIMINE_CONF_PATH "/boot/limine/limine.conf"
#define LIMINE_CONF_BUFSIZE 4096

int slot_write_limine_conf(char slot)
{
    if (slot != 'A' && slot != 'B') return -1;

    int target_entry = (slot == 'A') ? 0 : 1;
	(void)target_entry;

    return 0;
}
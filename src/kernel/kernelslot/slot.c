#include <kernel/kernelslot/slot.h>
#include <string/string.h>   // falls nötig
#include <string/log.h>

void dualslotvalidating(void)
{
    int fd = fs_open(SLOT_PATH, O_CREAT | O_WRONLY);
    if (fd >= 0) {
        const char *default_config = "A";
        fs_write(fd, default_config, str_len(default_config));
        fs_close(fd);
    }
}

char readslot(void)
{
    char buf[4];
    int fd = fs_open(SLOT_PATH, O_RDONLY);
    if (fd < 0)
        return 'A';

    int r = fs_read(fd, buf, sizeof(buf) - 1);
    fs_close(fd);

    if (r <= 0)
        return 'A';

    buf[r] = 0;
    str_to_upper(buf);

    if (str_equals(buf, "A"))
        return 'A';

    if (str_equals(buf, "B"))
        return 'B';

    return 'A';
}

int writeslot(char slot)
{
    if (slot != 'A' && slot != 'B')
        return -1;

    char buf[1];
    buf[0] = slot;

    int fd = fs_open(SLOT_PATH, O_WRONLY);
    if (fd < 0)
        return -1;

    fs_write(fd, buf, 1);
    fs_close(fd);

    cpu_poweroff(POWEROFF_REBOOT);

    return 0;
}

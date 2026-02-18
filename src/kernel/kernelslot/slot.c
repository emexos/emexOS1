#include <kernel/kernelslot/slot.h>
#include <string/string.h>
#include <kernel/modules/limine.h>

char readslot(void)
{
    char buf[4];
    buf[0] = '\0';
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
    buf[0] = '\0';
    buf[0] = slot;

    int fd = fs_open(SLOT_PATH, O_WRONLY);
    if (fd < 0)
        return -1;

    fs_write(fd, buf, 1);
    fs_close(fd);

    cpu_poweroff(POWEROFF_REBOOT);

    return 0;
}


void dualslotvalidating(void)
{
    // looks for the slot file
    int fd = fs_open(SLOT_PATH, O_RDONLY);
    if (fd < 0) {
        // if it doesnt exist write the file with content: "A"
        log("[SLOT]","active slot file not found, creating activeslot.txt...\n", d);

        fd = fs_open(SLOT_PATH, O_CREAT | O_WRONLY);
        if (fd >= 0) {
            const char *default_config = "A";
            fs_write(fd, default_config, str_len(default_config));
            fs_close(fd);
            log("[SLOT]","Default slot 'A' was set.\n", d);
        } else {
            log("[SLOT]","Error: Could not create slot file!\n", error);
            return; // abort if the file can't be created
        }
    } else {
        fs_close(fd);
        log("[SLOT]","Slot file found, checking contents...\n", d);
    }

    char active_slot = readslot();
    char buf[16];
    buf[0] = '\0';
    str_append_char(buf, active_slot);
    log("[SLOT]","read active slot... \n", d);
    log("[SLOT]","active slot: ", d);
    BOOTUP_PRINT(buf, white());
    BOOTUP_PRINT("\n", white());
}

#include "users.h"
#include <kernel/file_systems/vfs/vfs.h>
#include <string/string.h>
#include <memory/main.h>
#include <kernel/communication/serial.h>
#include <kernel/graph/theme.h>
#include <theme/doccr.h>

u32 g_current_uid = 0;
u32 g_current_gid = 0;

static user_entry_t user_table[USERS_MAX];
static u32 user_count = 0;

// uh..
static u32 parse_uint(const char *s)
{
    u32 v = 0;
    while (*s >= '0' && *s <= '9')
    {
        v = v * 10 + (u32)(*s - '0');
        s++;
    }
    return v;
}

static void parse_line(const char *line)
{
    // format is username:uid:gid:homedir
    char buf[256];
    int len = str_len(line);

    if (!line || !line[0] || line[0] == '#') return;
    if (len <= 0 || len >= 255) return;
    if (user_count >= USERS_MAX) return;
    while (*line == ' ' || *line == '\t') line++;
    while (len > 0 && (line[len-1] == ' ' || line[len-1] == '\t')) len--;

    for (int i = 0; i < len; i++) buf[i] = line[i];
    buf[len] = '\0';

    // split by colon into 4 fields
    char *f[4];
    int fi = 0;
    f[0] = buf;

    for (int i = 0; buf[i] && fi < 3; i++)
    {
        if (buf[i] == ':')
        {
            buf[i] = '\0';
            f[++fi] = &buf[i + 1];
        }
    }

    if (fi < 3) return;

    user_entry_t *u = &user_table[user_count];
    str_copy(u->username, f[0]);
    u->uid = parse_uint(f[1]);
    u->gid = parse_uint(f[2]);
    str_copy(u->home, f[3]);
    u->valid = 1;
    user_count++;
}

void users_init(void)
{
    memset(user_table, 0, sizeof(user_table));
    user_count = 0;
    g_current_uid = 0;
    g_current_gid = 0;
}

int users_load(const char *path)
{
    int fd = fs_open(path, O_RDONLY);
    if (fd < 0)
    {
        log("[USERS]", "users file not found\n", warning);
        return -1;
    }

    char buf[2048];
    ssize_t n = fs_read(fd, buf, sizeof(buf) - 1);
    fs_close(fd);

    if (n <= 0) return -1;
    buf[n] = '\0';

    char line[256];
    int li = 0;

    for (int i = 0; i <= (int)n; i++)
    {
        if (buf[i] == '\n' || buf[i] == '\0')
        {
            line[li] = '\0';
            if (li > 0) parse_line(line);
            li = 0;
        } else {
            if (li < 255) line[li++] = buf[i];
        }
    }

    char cbuf[32];
    cbuf[0] = '\0';
    str_append_uint(cbuf, user_count);
    log("[USERS]", "loaded ", d);
    BOOTUP_PRINT(cbuf, white());
    BOOTUP_PRINT(" user(s)\n", white());

    return 0;
}

user_entry_t *users_get_by_uid(u32 uid)
{
    for (u32 i = 0; i < user_count; i++)
    {
        if (user_table[i].valid && user_table[i].uid == uid)return &user_table[i];
    }
    return NULL;
}

user_entry_t *users_get_by_name(const char *name)
{
    if (!name) return NULL;
    for (u32 i = 0; i < user_count; i++)
    {
        if (user_table[i].valid && str_equals(user_table[i].username, name))return &user_table[i];
    }
    return NULL;
}

user_entry_t *users_get_first_regular(void)
{
    for (u32 i = 0; i < user_count; i++)
    {
        if (user_table[i].valid && user_table[i].uid != UID_ROOT)return &user_table[i];
    }
    return NULL;
}

u32 users_get_count(void) {
    return user_count;
}

void users_setup_homes(void)
{
	/* both should be already there cuz of initrd but still make sure */
    fs_mkdir("/root");
    fs_mkdir("/home");

    for (u32 i = 0; i < user_count; i++)
    {
        if (!user_table[i].valid) continue;
        if (user_table[i].uid == UID_ROOT) continue;
        fs_mkdir(user_table[i].home);
    }
}
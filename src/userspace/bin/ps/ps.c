#include <stdio.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

static int is_num(const char *s)
{
    if (!s || !s[0]) return 0;

    for (int i = 0; s[i]; i++) {
        if (s[i] < '0' || s[i] > '9') return 0;
    }
    return 1;
}

static void parse_field(const char *buf, const char *key, char *out, int outsz)
{
    int klen = (int)strlen(key);
    const char *p = buf;
    out[0] = '\0';
    while (*p) {
        if (strncmp(p, key, (size_t)klen) == 0) {
            p += klen;
            while (*p == ' ') p++;
            int i = 0;
            while (*p && *p != '\n' && i < outsz - 1) out[i++] = *p++;
            out[i] = '\0';
            return;
        }
        while (*p && *p != '\n') p++;
        if (*p) p++;
    }
}

int main(void)
{
    DIR *dir = opendir("/proc");
    if (!dir) {
        printf("ps: cannot open /proc\n");
        return 1;
    }

    printf("PID    STATE      NAME\n");

    struct dirent *e;

    while ((e = readdir(dir)))
    {
        if (!is_num(e->d_name)) continue;

        char path[128];
        snprintf(path, sizeof(path), "/proc/%s/status", e->d_name);

        int fd = open(path, O_RDONLY);
        if (fd < 0) continue;

        char buf[256];
        int n = (int)read(fd, buf, sizeof(buf) - 1);
        close(fd);

        if (n <= 0) continue;
        buf[n] = '\0';

        char name[64], state[16];
        parse_field(buf, "name:", name, sizeof(name));
        parse_field(buf, "state:", state, sizeof(state));

        printf("%-7s%-11s%s\n", e->d_name, state, name);
    }

    closedir(dir);
    return 0;
}
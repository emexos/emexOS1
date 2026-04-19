#include "libdesktop.h"

#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define DT_CMD    "/tmp/dt/cmd"
#define DT_DIRTY  "/tmp/dt/dirty"
#define DT_CURSOR "/tmp/dt/cursor"


static int _slen(const char *s) {
    int n = 0; while (s[n]) n++; return n;
}

static void _int_to_str(int v, char *out)
{
    char tmp[16];
    int i = 0;
    int j = 0;
    int neg = (v < 0);

    if (v == 0) { out[0] = '0'; out[1] = '\0'; return; }
    if (neg) v = -v;
    while (v) { tmp[i++] = '0' + v % 10; v /= 10; }
    if (neg) tmp[i++] = '-';
    while (i > 0) out[j++] = tmp[--i];

    out[j] = '\0';
}

static void _app_int(char *buf, int *pos, int v)
{
    char tmp[12];
    int i = 0;
    _int_to_str(v, tmp);
    while (tmp[i]) buf[(*pos)++] = tmp[i++];
}

static void _app_str(char *buf, int *pos, const char *s) {
    while (*s) buf[(*pos)++] = *s++;
}

// handles structural commands (OCTM)
static void _cmd_append(const char *line)
{
    static char existing[4096];
    int elen = 0;
    int actual = 0;
    int fd = open(DT_CMD, O_RDONLY);

    if (fd >= 0)
    {
        int r = (int)read(fd, existing, sizeof(existing) - 1);
        close(fd);

        if (r > 0)
        {
            existing[r] = '\0';
            while (actual < r && existing[actual] != '\0') actual++;
            elen = actual;
        }
    }

    fd = open(DT_CMD, O_WRONLY | O_CREAT);
    if (fd < 0) return;
    if (elen > 0) write(fd, existing, (unsigned)elen);
    write(fd, line, (unsigned)_slen(line));
    close(fd);
}


static int _createWindow(
	const char *title,
    int x, int y, int w, int h,
    unsigned int style
){
    /* fmt: O pid style x y w h title\n */
    char buf[256];
    int  p = 0;
    buf[p++] = 'O'; buf[p++] = ' ';
    _app_int(buf, &p, (int)getpid()); buf[p++] = ' ';
    _app_int(buf, &p, (int)style);    buf[p++] = ' ';
    _app_int(buf, &p, x);             buf[p++] = ' ';
    _app_int(buf, &p, y);             buf[p++] = ' ';
    _app_int(buf, &p, w);             buf[p++] = ' ';
    _app_int(buf, &p, h);             buf[p++] = ' ';
    _app_str(buf, &p, title);
    buf[p++] = '\n';
    buf[p]   = '\0';
    _cmd_append(buf);
    return 0;
}

static void _closeWindow(void)
{
    /* fmt: C pid\n */
    char buf[32];
    int  p = 0;
    buf[p++] = 'C'; buf[p++] = ' ';
    _app_int(buf, &p, (int)getpid());
    buf[p++] = '\n';
    buf[p] = '\0';
    _cmd_append(buf);
}

static void _setTitle(const char *title)
{
    /* fmt: T pid title\n */
    char buf[256];
    int  p = 0;

    buf[p++] = 'T'; buf[p++] = ' ';
    _app_int(buf, &p, (int)getpid()); buf[p++] = ' ';
    _app_str(buf, &p, title);
    buf[p++] = '\n';
    buf[p]= '\0';
    _cmd_append(buf);
}

static void _markDirty(void)
{
    int fd = open(DT_DIRTY, O_WRONLY | O_CREAT);
    if (fd >= 0) { write(fd, "1\n", 2); close(fd); }
}

static int _getCursor(int *x, int *y)
{
    char buf[64];
    int vx = 0;
    int vy = 0;
    int fd = open(DT_CURSOR, O_RDONLY);
    int n = (int)read(fd, buf, sizeof(buf) - 1);

    if (fd < 0) return -1;
    close(fd);
    if (n <= 0) return -1;

    buf[n] = '\0';

    const char *p = buf;

    while (*p >= '0' && *p <= '9') vx = vx * 10 + (*p++ - '0');
    while (*p == ' ') p++;
    while (*p >= '0' && *p <= '9') vy = vy * 10 + (*p++ - '0');

    *x = vx;
    *y = vy;
    return 0;
}

// for "desktop."
Desktop desktop = {
    .createWindow = _createWindow,
    .closeWindow  = _closeWindow,
    .setTitle     = _setTitle,
    .markDirty    = _markDirty,
    .getCursor    = _getCursor,
};
#include "cmd.h"
#include "../config/cfg.h"
#include "../win/win.h"

#include <unistd.h>
#include <fcntl.h>

static int str_to_int(const char *s)
{
    int neg = 0, v = 0;
    while (*s == ' ') s++;
    if (*s == '-') { neg = 1; s++; }
    while (*s >= '0' && *s <= '9') v = v * 10 + (*s++ - '0');
    return neg ? -v : v;
}

static const char *next_tok(const char **p)
{
    static char tok[DT_TITLE_MAX];
    int i = 0;
    while (**p == ' ') (*p)++;
    while (**p && **p != ' ' && **p != '\n' && i < DT_TITLE_MAX - 1)
        tok[i++] = *(*p)++;
    tok[i] = '\0';
    return tok;
}

static void clear_file(const char *path, int n)
{
    if (n <= 0) return;
    int fd = open(path, O_WRONLY | O_CREAT);
    if (fd < 0) return;
    static char clr[4096];
    int cnt = n < 4096 ? n : 4096;
    for (int i = 0; i < cnt; i++) clr[i] = '\n';
    write(fd, clr, (unsigned)cnt);
    close(fd);
}

static void process_line(const char *line, cmd_result_t *result)
{
    if (!line[0] || line[0] == '\n') return;
    char cmd = line[0];
    const char *p = line + 1;

    if (cmd == 'O')
    {
        pid_t pid = (pid_t)str_to_int(next_tok(&p));
        unsigned int style = (unsigned int)str_to_int(next_tok(&p));

        char title[DT_TITLE_MAX];

        int x = str_to_int(next_tok(&p)), y = str_to_int(next_tok(&p));
        int w = str_to_int(next_tok(&p)), h = str_to_int(next_tok(&p));
        int ti = 0;


        while (*p == ' ') p++;
        while (*p && *p != '\n' && ti < DT_TITLE_MAX - 1) title[ti++] = *p++;

        title[ti] = '\0';

        int idx = win_add(pid, title, x, y, w, h, style);
        win_focus(idx);
        result->win_changed = 1;

    } else if (cmd == 'C')
    {
        pid_t pid = (pid_t)str_to_int(next_tok(&p));
        win_remove(pid);
        result->win_changed = 1;

    } else if (cmd == 'T')
    {
        pid_t pid = (pid_t)str_to_int(next_tok(&p));
        char title[DT_TITLE_MAX];
        int ti = 0;

        while (*p == ' ') p++;
        while (*p && *p != '\n' && ti < DT_TITLE_MAX - 1) title[ti++] = *p++;
        title[ti] = '\0';

        win_set_title(pid, title);
        result->win_changed = 1;

    } else if (cmd == 'M')
    {
        pid_t pid = (pid_t)str_to_int(next_tok(&p));
        int nx = str_to_int(next_tok(&p)), ny = str_to_int(next_tok(&p));
        int idx = win_find_pid(pid);

        win_move(idx, nx, ny);
        result->win_changed = 1;
    }
}

void cmd_process(cmd_result_t *result)
{
    static char buf[4096];
    //static char zeros[4096];
    result->win_changed = 0;

    int fd = open(DT_CMD, O_RDONLY);
    int n = (int)read(fd, buf, sizeof(buf) - 1);
    if (fd < 0) return; /* if file exists */

    close(fd); if (n <= 0) return;
    buf[n] = '\0';
    if (buf[0] == '\0') return;

    clear_file(DT_CMD, n);

    /* process*/
    const char *line = buf;
    while (*line) {
        process_line(line, result);
        while (*line && *line != '\n') line++;
        if (*line == '\n') line++;
    }
}

int cmd_check_dirty(void)
{
    char buf[4];
    int fd = open(DT_DIRTY, O_RDONLY);
    int n = (int)read(fd, buf, 1);

    if (fd < 0) return 0;
    close(fd);
    if (n <= 0 || buf[0] != '1') return 0;

    fd = open(DT_DIRTY, O_WRONLY | O_CREAT);
    if (fd >= 0)
    {
        write(fd, "\n\n", 2);
        close(fd);
    }
    return 1;
}
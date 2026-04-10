#include "../vfs.h"
#include "procfs.h"
#include <memory/main.h>
#include <string/string.h>
#include <kernel/mem/klime/klime.h>
#include <kernel/arch/x86_64/exceptions/timer.h>
#include <kernel/cpu/cpu.h>
#include <config/system.h>

#if ENABLE_ULIME
#include <kernel/user/ulime.h>
extern ulime_t *ulime;
#endif

/*e
 *
 * NOTE:
 * this is just a stub rn, i need those folders in /proc for porting software,
 * in future they will work ofc.
 *
 */
extern void *fs_klime;

static void ap_u64(char *buf, u64 v)
{
    if (v == 0) { str_append_char(buf, '0'); return; }
    char rev[24]; int i = 0;
    while (v > 0) { rev[i++] = '0' + (int)(v % 10); v /= 10; }
    for (int j = i - 1; j >= 0; j--) str_append_char(buf, rev[j]);
}
static void ap_kb(char *buf, const char *label, u64 kb)
{
    str_append(buf, label);
    ap_u64(buf, kb);
    str_append(buf, " kB\n");
}
static void count_procs(u64 *out_count, u64 *out_last_pid)
{
    *out_count   = 1;
    *out_last_pid = 1;

	#if ENABLE_ULIME
	    if (!ulime) return;

	    ulime_proc_t *p = ulime->ptr_proc_list;
	    u64 n = 0;

	    while (p) { n++; *out_last_pid = p->pid; p = p->next; }
	    *out_count = n;
	#endif
}

static char *gen_uptime(u64 *out_len)
{
    char *buf = (char *)klime_create((klime_t *)fs_klime, 64);
    if (!buf) return NULL;
    buf[0] = '\0';

    u64 up = timer_get_uptime_seconds();
    ap_u64(buf, up);
    str_append(buf, ".00 ");
    ap_u64(buf, up);/*idle */
    str_append(buf, ".00\n");

    *out_len = (u64)str_len(buf);
    return buf;
}

static char *gen_stat(u64 *out_len)
{
    char *buf = (char *)klime_create((klime_t *)fs_klime, 512);
    if (!buf) return NULL;
    buf[0] = '\0';

    u64 idle = timer_get_uptime_seconds() * 100;

    str_append(buf, "cpu  0 0 0 "); ap_u64(buf, idle); str_append(buf, " 0 0 0 0 0 0\n");
    str_append(buf, "cpu0 0 0 0 "); ap_u64(buf, idle); str_append(buf, " 0 0 0 0 0 0\n");
    str_append(buf, "intr 0\n");
    str_append(buf, "ctxt 0\n");
    str_append(buf, "btime 0\n");   /* boot time*/

    u64 nproc, last_pid;
    count_procs(&nproc, &last_pid);

    str_append(buf, "processes "); ap_u64(buf, nproc); str_append(buf, "\n");
    str_append(buf, "procs_running 1\n");
    str_append(buf, "procs_blocked 0\n");

    *out_len = (u64)str_len(buf);
    return buf;
}

static char *gen_meminfo(u64 *out_len)
{
    char *buf = (char *)klime_create((klime_t *)fs_klime, 512);
    if (!buf) return NULL;
    buf[0] = '\0';

    /*
     * 1046528kb; 1047504kb
     */

    /* physmem_get_total() returns total frames, physmem_free_get() returns free frames */
    u64 total_gb = ((u64)physmem_get_total() * PAGE_SIZE) / (1024 * 1024 * 1024); /* GB */
    u64 total_mb = ((u64)physmem_get_total() * PAGE_SIZE) / (1024 * 1024);    	  /* MB */
    u64 total_kb = (	 physmem_get_total() * PAGE_SIZE) / 1024; 				  /* KB */
    u64 free_kb  = (	 physmem_free_get()  * PAGE_SIZE) / 1024; 				  /* KB */

    ap_kb(buf, "MemTotal:      ", total_kb);
    ap_kb(buf, "MemFree:       ", free_kb);
    ap_kb(buf, "MemAvailable:  ", free_kb);
    ap_kb(buf, "Buffers:       ", 0);
    ap_kb(buf, "Cached:        ", 0);
    ap_kb(buf, "SwapCached:    ", 0);
    ap_kb(buf, "Active:        ", 0);
    ap_kb(buf, "Inactive:      ", 0);
    ap_kb(buf, "SwapTotal:     ", 0);
    ap_kb(buf, "SwapFree:      ", 0);

    *out_len = (u64)str_len(buf);
    return buf;
}
static char *gen_cpuinfo(u64 *out_len)
{
	//cpu_get_vendor(void);
	// TODO: FINISHH1111!!!!!!!!!!!!!!! TOMOWOOOOOOOOOWOWOWOWOWOWOWOWOWOO
}

static char *gen_loadavg(u64 *out_len)
{
    char *buf = (char *)klime_create((klime_t *)fs_klime, 64);
    if (!buf) return NULL;
    buf[0] = '\0';

    u64 nproc, last_pid;
    count_procs(&nproc, &last_pid);

    str_append(buf, "0.00 0.00 0.00 1/");
    ap_u64(buf, nproc);
    str_append(buf, " ");
    ap_u64(buf, last_pid);
    str_append(buf, "\n");

    *out_len = (u64)str_len(buf);
    return buf;
}

static int dir_open(fs_node *node, fs_file *file)
{
    procfs_data *d = (procfs_data *)node->priv;
    if (!d) return -1;
    (void)file;

    char *content = NULL;
    u64 len = 0;

    switch (d->kind) {
        case PROCFS_KIND_UPTIME: content = gen_uptime(&len); break;
        case PROCFS_KIND_STAT: content = gen_stat(&len); break;
        case PROCFS_KIND_MEMINFO: content = gen_meminfo(&len); break;
        case PROCFS_KIND_LOADAVG: content = gen_loadavg(&len); break;
        default: return -1;
    }

    if (!content) return -1;
    d->buf = content;
    d->len = len;

    return 0;
}

static int dir_close(fs_file *file) { (void)file; return 0; }

static ssize_t dir_read(fs_file *file, void *buf, size_t cnt)
{
    procfs_data *d = (procfs_data *)file->node->priv;
    size_t to_read = cnt;
    if (!d || !d->buf) return 0;
    if (file->pos >= d->len) return 0;
    if (file->pos + to_read > d->len) to_read = d->len - file->pos;

    memcpy(buf, d->buf + file->pos, to_read);
    file->pos += (u64)to_read;
    return (ssize_t)to_read;
}

static ssize_t dir_write(fs_file *f, const void *b, size_t n)
{
    (void)f; (void)b; (void)n;
    return -1;
}

static fs_ops procfs_dir_ops =
{
    .open    = dir_open,
    .close   = dir_close,
    .read    = dir_read,
    .write   = dir_write,
    .lookup  = NULL,
    .create  = NULL,
    .mkdir   = NULL,
    .readdir = NULL,
};

fs_node *procfs_dir_node(const char *name, u8 kind)
{
    fs_node *node = fs_mknode(name, FS_FILE);
    if (!node) return NULL;

    procfs_data *data = (procfs_data *)klime_create((klime_t *)fs_klime, sizeof(procfs_data));
    if (!data) return NULL;

    data->pid = 0;
    data->kind = kind;
    data->buf = NULL;
    data->len = 0;

    node->priv = data;
    node->ops  = &procfs_dir_ops;
    return node;
}
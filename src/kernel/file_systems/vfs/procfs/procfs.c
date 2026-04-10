#include "../vfs.h"
#include "procfs.h"
#include <memory/main.h>
#include <string/string.h>
#include <kernel/mem/klime/klime.h>
#include <kernel/user/ulime.h>
#include <theme/stdclrs.h>
#include <theme/doccr.h>
#include <config/system.h>

//
// procfs:
//  layout:
//    /proc/<pid>/status
//    /proc/<pid>/maps
//    /proc/stat
//    /proc/uptime
//    /proc/meminfo
//    /proc/loadavg
//

#if ENABLE_ULIME
extern ulime_t *ulime;
#endif

#define STATUS_MSG "status"
#define MAPS_MSG "maps"

static fs_node *procfs_root = NULL; // nothings mounted on start

static u64 procfs_atopid(const char *s)
{
    if (!s || *s == '\0') return 0;
    u64 n = 0;
    const char *p = s;
    while (*p >= '0' && *p <= '9') {
        n = n * 10 + (u64)(*p - '0');
        p++;
    }
    // must consume the whole string
    if (*p != '\0') return 0;
    if (p == s) return 0;

    return n;
}
static ulime_proc_t *procfs_find_pid(u64 pid)
{
    #if ENABLE_ULIME
        if (!ulime) return NULL;
        ulime_proc_t *proc = ulime->ptr_proc_list;
        while (proc) {
            if (proc->pid == pid) return proc;
            proc = proc->next;
        }
    #else
        (void)pid;
    #endif
    return NULL;
}

static const char *procfs_state_str(u64 state)
{
    switch (state) {
        case PROC_CREATED: return "CREATED";
        case PROC_READY:   return "READY";
        case PROC_RUNNING: return "RUNNING";
        case PROC_BLOCKED: return "BLOCKED";
        case PROC_ZOMBIE:  return "ZOMBIE";
        default: return "UNKNOWN";
    }
}
static char *procfs_gen_status(ulime_proc_t *proc, u64 *out_len)
{
    char *buf = (char *)klime_create((klime_t *)fs_klime, 256);
    char tmp[32];
    if (!buf) return NULL;

    str_copy(buf,  "name:     ");
    str_append(buf,(char *)proc->name);
    str_append(buf,"\n");
    str_append(buf,"pid:      ");
    tmp[0] = '\0';
    str_append_uint(tmp, (u32)proc->pid);
    str_append(buf, tmp);
    str_append(buf, "\n");
    str_append(buf,"state:    ");
    str_append(buf,procfs_state_str(proc->state));
    str_append(buf,"\n");
    str_append(buf,"priority: ");
    tmp[0] = '\0';
    str_append_uint(tmp, (u32)proc->priority);
    str_append(buf,tmp);
    str_append(buf,"\n");

    *out_len = (u64)str_len(buf);
    return buf;
}
static char *procfs_gen_maps(ulime_proc_t *proc, u64 *out_len)
{
    char *buf = (char *)klime_create((klime_t *)fs_klime, 256);
    char tmp[32];
    if (!buf) return NULL;

    str_copy(buf,  "heap:  0x");
    str_from_hex(tmp, proc->heap_base);
    str_append(buf,tmp);
    str_append(buf,"  size: ");
    tmp[0] = '\0';
    str_append_uint(tmp, (u32)proc->heap_size);
    str_append(buf,tmp);
    str_append(buf,"\n");
    str_append(buf,"stack: 0x");
    str_from_hex(tmp, proc->stack_base);
    str_append(buf, tmp);
    str_append(buf,"  size: ");
    tmp[0] = '\0';
    str_append_uint(tmp, (u32)proc->stack_size);
    str_append(buf,tmp);
    str_append(buf,"\n");

    *out_len = (u64)str_len(buf);
    return buf;
}
static fs_node *procfs_mknode(const char *name, u8 type, u64 pid, u8 kind)
{
    fs_node *node = fs_mknode(name, type);
    if (!node) return NULL;

    procfs_data *data = (procfs_data *)klime_create((klime_t *)fs_klime, sizeof(procfs_data));
    if (!data) return NULL;

    data->pid  = pid;
    data->kind = kind;
    data->buf  = NULL;
    data->len  = 0;

    node->priv = data;
    return node;
}

// ops


static int procfs_open(fs_node *node, fs_file *file)
{
    procfs_data *data = (procfs_data *)node->priv;
    if (!data) return -1;

    (void)file;

    // generate content for file nodes on open
    if (data->kind == PROCFS_KIND_STATUS || data->kind == PROCFS_KIND_MAPS) {
        #if ENABLE_ULIME // why did i just make this enable_ulime...
        // the problem it would take days to remove it its toooo deep in the system...
            ulime_proc_t *proc = procfs_find_pid(data->pid);
            if (!proc) return -1;

            if (data->kind == PROCFS_KIND_STATUS) {
                data->buf = procfs_gen_status(proc, &data->len);
            } else {
                data->buf = procfs_gen_maps(proc, &data->len);
            }

            if (!data->buf) return -1;
        #else
            data->buf = (char *)klime_create((klime_t *)fs_klime, 32);
            if (!data->buf) return -1;
            str_copy(data->buf, "ulime disabled\n");
            data->len = (u64)str_len(data->buf);
        #endif
    }

    return 0;
}

static int procfs_close(fs_file *file) {(void)file; return 0;}

static ssize_t procfs_read(fs_file *file, void *buf, size_t cnt)
{
    fs_node *node = file->node;
    procfs_data *data = (procfs_data *)node->priv;

    // if dirs are not readable
    if (!data || data->kind == PROCFS_KIND_ROOT || data->kind == PROCFS_KIND_DIR) return -1;
    if (!data->buf) return 0;

    // eof
    if (file->pos >= data->len) return 0;

    size_t to_read = cnt;
    if (file->pos + to_read > data->len) {
        to_read = data->len - file->pos;
    }

    memcpy(buf, data->buf + file->pos, to_read);
    file->pos += to_read;

    return (ssize_t)to_read;
}

static ssize_t procfs_write(fs_file *f, const void *b, size_t n)
{
    (void)f; (void)b; (void)n;
    return -1;
}

static fs_ops procfs_ops;

static fs_node *procfs_readdir(fs_node *dir)
{
    if (!dir || !dir->priv) return NULL;
    procfs_data *data = (procfs_data *)dir->priv;

	#if ENABLE_ULIME
	    if (data->kind == PROCFS_KIND_ROOT)
	    {
	        /* global /proc/* files come first in the listing */
	        fs_node *g_stat    = procfs_dir_node("stat", 	PROCFS_KIND_STAT   );
	        fs_node *g_uptime  = procfs_dir_node("uptime", 	PROCFS_KIND_UPTIME );
	        fs_node *g_meminfo = procfs_dir_node("meminfo", PROCFS_KIND_MEMINFO);
	        fs_node *g_loadavg = procfs_dir_node("loadavg", PROCFS_KIND_LOADAVG);
			fs_node *g_cpuinfo = procfs_dir_node("cpuinfo", PROCFS_KIND_CPUINFO);

	        if (g_stat) g_stat->next = g_uptime;
	        if (g_uptime) g_uptime->next = g_meminfo;
	        if (g_meminfo) g_meminfo->next = g_loadavg;


	        /* pid dirs come after */
	        if (!ulime) return g_stat;

	        fs_node *pid_head = NULL;
	        fs_node *tail = NULL;

	        ulime_proc_t *proc = ulime->ptr_proc_list;

	        while (proc)
	        {
	            char pidstr[24];
	            u64 pid = proc->pid;
	            int ti = 0;
	            char tmp[24];

	            if (pid == 0) { pidstr[0] = '0'; pidstr[1] = '\0'; }
	            else
	            {
	                while (pid > 0) { tmp[ti++] = '0' + (int)(pid % 10); pid /= 10; }
	                for (int i = 0; i < ti; i++) pidstr[i] = tmp[ti - 1 - i];
	                pidstr[ti] = '\0';
	            }

	            fs_node *n = procfs_mknode(pidstr, FS_DIR, proc->pid, PROCFS_KIND_DIR);
	            if (n)
	            {
	                n->ops = &procfs_ops;
	                if (!pid_head) pid_head = n;
	                else tail->next = n;
	                tail = n;
	            }
	            proc = proc->next;
	        }

	        if (g_loadavg) g_loadavg->next = pid_head;
	        else if (g_meminfo) g_meminfo->next = pid_head;
	        else if (g_uptime) g_uptime->next = pid_head;
	        else if (g_stat) g_stat->next = pid_head;

	        return g_stat ? g_stat : pid_head;
	    }

	    if (data->kind == PROCFS_KIND_DIR)
	    {
	        fs_node *status = procfs_mknode(STATUS_MSG, FS_FILE, data->pid, PROCFS_KIND_STATUS);
	        fs_node *maps   = procfs_mknode(MAPS_MSG,   FS_FILE, data->pid, PROCFS_KIND_MAPS);

	        if (status) status->ops = &procfs_ops;
	        if (maps) maps->ops = &procfs_ops;
	        if (status && maps) status->next = maps;

	        return status ? status : maps;
	    }
	#endif

    return NULL;
}

static fs_node *procfs_lookup(fs_node *dir, const char *name)
{
    if (dir->type != FS_DIR) return NULL;

    procfs_data *data = (procfs_data *)dir->priv;
    if (!data) return NULL;

    if (data->kind == PROCFS_KIND_ROOT)
    {
        if (str_equals(name, "stat")) return procfs_dir_node("stat", PROCFS_KIND_STAT);
        if (str_equals(name, "uptime")) return procfs_dir_node("uptime", PROCFS_KIND_UPTIME);
        if (str_equals(name, "meminfo")) return procfs_dir_node("meminfo", PROCFS_KIND_MEMINFO);
        if (str_equals(name, "loadavg")) return procfs_dir_node("loadavg", PROCFS_KIND_LOADAVG);

        u64 pid = procfs_atopid(name);
        if (pid == 0) return NULL;
        if (!procfs_find_pid(pid)) return NULL;

        fs_node *piddir = procfs_mknode(name, FS_DIR, pid, PROCFS_KIND_DIR);
        if (!piddir) return NULL;
        piddir->ops = dir->ops;
        return piddir;
    }

    if (data->kind == PROCFS_KIND_DIR)
    {
        u8 kind = 0;
        if (str_equals(name, STATUS_MSG))   kind = PROCFS_KIND_STATUS;
        else if (str_equals(name, MAPS_MSG)) kind = PROCFS_KIND_MAPS;
        else return NULL;

        fs_node *fnode = procfs_mknode(name, FS_FILE, data->pid, kind);
        if (!fnode) return NULL;
        fnode->ops = dir->ops;
        return fnode;
    }

    return NULL;
}

static fs_ops procfs_ops = {
    .open    = procfs_open,
    .close   = procfs_close,
    .read    = procfs_read,
    .write   = procfs_write,
    .lookup  = procfs_lookup,
    .create  = NULL,
    .mkdir   = NULL,
    .readdir = procfs_readdir,
};

//
// mount
//

static int procfs_mount(const char *src, const char *tgt, fs_mnt *mnt)
{
    {
        (void)src;
        (void)tgt;
    }

    fs_node *root = procfs_mknode("proc", FS_DIR, 0, PROCFS_KIND_ROOT);
    if (!root) return -1;

    root->ops = &procfs_ops;
    mnt->root = root;
    procfs_root = root;

    return 0;
}

static fs_type procfs_type =
{
    .name  = "procfs",
    .mount = procfs_mount,
    .ops   = &procfs_ops,
};

// register procfs type
void procfs_register(void)
{
    fs_register(&procfs_type);
}
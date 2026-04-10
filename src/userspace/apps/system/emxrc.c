#include "emxrc.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

static char buf[2048];
static char *tp;
static char tb[256]; // token result buffer

#define PREFIX ":: emxrc: "
#define CMD_PREFIX '-'

#define VAR_NAME "var"
#define EMX_NAME "ep"
#define ELF_NAME "elf"
#define EXE_NAME "exec"
#define WAI_NAME "wait"
#define FMT_NAME "f"

//#define BG " &"
#define BG ""

static char *tok(void)
{
 	int i = 0;

    while (*tp == ' ' || *tp == '\t') tp++;

    if (!*tp || *tp == '\n') return NULL;
    if (*tp == '"') {
        for (tp++; *tp && *tp != '"' && *tp != '\n';) tb[i++] = *tp++;
        if (*tp == '"') tp++;
        tb[i] = '\0';
        return tb;
    }
    if (*tp == '=' || *tp == '(' || *tp == ')' || *tp == ':') {
        tb[0] = *tp++; tb[1] = '\0';
        return tb;
    }

    while (*tp
    	&& *tp != ' ' && *tp != '\t' && *tp != '\n'
        && *tp != '=' && *tp != '('  && *tp != ')'
        && *tp != ':' && *tp != '"'  && i   < 255
    ) tb[i++] = *tp++;

    tb[i] = '\0';
    return i ? tb : NULL;
}

static void skipline(void) {
    while (*tp && *tp != '\n') tp++;
    if (*tp) tp++;
}
static void emx_sleep(int ticks)
{
    volatile int i, j;
    for (i = 0; i < ticks; i++) {
        for (j = 0; j < 1000000; j++);
    }
}
static int is_direct_path(const char *s) {
    return s && (s[0] == '/' || s[0] == '.');
}

int emxrc_parse(const char *path, emxrc_t *out)
{
    if (!out) return -1;
    out->var_count = out->exec_count = 0;

    int fd = open(path, O_RDONLY);
    int n = (int)read(fd, buf, sizeof(buf) - 1);

    if (fd < 0) return -1;
    close(fd);
    if (n <= 0) return -1;
    buf[n] = '\0';
    tp = buf;

    while (*tp)
    {
        while (*tp == ' ' || *tp == '\t') tp++; // space or tab
        if (*tp == '\n') { tp++; continue; } // new line
        if (tp[0]=='/' && tp[1]=='/') { skipline(); continue; } // comments

        char *kw = tok();
        if (!kw) { skipline(); continue; }

        // var name=("path") or var name=(ep: "path")
        if (strcmp(kw, VAR_NAME) == 0 && out->var_count < EMXRC_MAX_VARS)
        {
            emxrc_var_t *v = &out->vars[out->var_count];
            v->fmt = EMXRC_FMT_ELF;

            char *name = tok(); if (!name) { skipline(); continue; }
            strncpy(v->name, name, sizeof(v->name) - 1);

            tok();
            tok(); // skip '=' '('

            char *next = tok();
            if (next && strcmp(next, EMX_NAME) == 0) {
                v->fmt = EMXRC_FMT_EP;
                tok(); // skip ':'
                next = tok();
            }
            if (next) strncpy(v->path, next, sizeof(v->path) - 1);
            out->var_count++;
        }
        //print("text")
        else if (strcmp(kw, "print") == 0 && out->exec_count < EMXRC_MAX_EXECS)
        {
        	emxrc_exec_t *e = &out->execs[out->exec_count];
         	memset(e, 0, sizeof(*e));

          	e->is_print= 1;
            tok(); // (
            char *msg = tok(); // "text"
            if (msg)
            {
            	strncpy(e->message, msg, sizeof(e->message)- 1);
            }
            out->exec_count++;
        }
        //elog("text")
        else if (strcmp(kw, "elog") == 0 && out->exec_count < EMXRC_MAX_EXECS)
        {
        	emxrc_exec_t *e = &out->execs[out->exec_count];
         	memset(e, 0, sizeof(*e));

          	e->is_elog = 1;
            tok(); // (
            char *msg = tok(); // "text"
            if (msg)
            {
            	strncpy(e->message, msg, sizeof(e->message)- 1);
            }
            out->exec_count++;
        }

        // wait <time>
        else if (strcmp(kw, WAI_NAME) == 0 && out->exec_count < EMXRC_MAX_EXECS)
        {
            emxrc_exec_t *e = &out->execs[out->exec_count];
            memset(e, 0, sizeof(*e));

            e->is_wait = 1;

            char *t = tok();
            if (t) {
                e->wait_time = atoi(t);
            }

            out->exec_count++;
        }

        // exec -f"format" var_name
        else if (strcmp(kw, EXE_NAME) == 0 && out->exec_count < EMXRC_MAX_EXECS)
        {
            emxrc_exec_t *e = &out->execs[out->exec_count];
            memset(e, 0, sizeof(*e));
            e->fmt = EMXRC_FMT_INHERIT; // fmt == format btw

            char *next = tok();
            if (!next) { skipline(); continue; }
            while (next && next[0] == CMD_PREFIX) {
                if (strcmp(next + 1, "bg") == 0) {
                    e->bg = 1;
                    next = tok();
                } else if (next[1] == FMT_NAME[0]) {
                    char *fs = tok();
                    e->fmt = (fs && strcmp(fs, EMX_NAME) == 0) ? EMXRC_FMT_EP : EMXRC_FMT_ELF;
                    next = tok();
                } else {
                    next = tok();
                }
            }

            if (!next) { skipline(); continue; }

            if (is_direct_path(next))
            {
                strncpy(e->direct_path, next, sizeof(e->direct_path) - 1);

                if (e->fmt == EMXRC_FMT_INHERIT) {
                    size_t l = strlen(next);
                    if (l > 4 && strcmp(next + l - 4, ".emx") == 0)
                        e->fmt = EMXRC_FMT_EP;
                    else
                        e->fmt = EMXRC_FMT_ELF;
                }
            } else {
                strncpy(e->var_name, next, sizeof(e->var_name) - 1);
            }

            out->exec_count++;
        }
        skipline();
    }
    return 0;
}

void emxrc_run(emxrc_t *rc)
{
	//printf("strt emexrc_run");
    for (int i = 0; i < rc->exec_count; i++) {
        emxrc_exec_t *e = &rc->execs[i];

   	    if (e->is_wait) {
	        //printf(PREFIX "wait %d\n", e->wait_time);
	        emx_sleep(e->wait_time);
	        continue;
	    }
        if (e->is_print)
        {
        	printf("%s\n", e->message);
        	continue;
        }
        if (e->is_elog)
        {
        	printf(PREFIX "%s\n", e->message);
         	continue;
        }

        const char *path = NULL;
        emxrc_fmt_t fmt = e->fmt;

        if (e->direct_path[0] != '\0') {
            path = e->direct_path;
        } else {
            for (int j = 0; j < rc->var_count; j++) {
                if (strcmp(rc->vars[j].name, e->var_name) == 0) {
                    path = rc->vars[j].path;
                    if (fmt == EMXRC_FMT_INHERIT) fmt = rc->vars[j].fmt;
                    break;
                }
            }
        }

        if (!path) {
            fprintf(stderr, PREFIX "unknown var '%s'\n", e->var_name);
            continue;
        }

        printf(
        	PREFIX EXE_NAME " %s (%s)%s\n",
         	path,
            fmt == EMXRC_FMT_EP ? EMX_NAME : ELF_NAME,
            e->bg ? BG : ""
        );

        char *const argv[] = { (char *)path, (char *)0 };
        char *const envp[] = { (char *)0 };

        pid_t p = fork();
        if (p == 0) {
            execve(path, argv, envp);
            _exit(1); // execve failed
        }

        if (!e->bg) {
            waitpid(p, NULL, 0);
        }
    }
    //printf("end emexrc_run");
}

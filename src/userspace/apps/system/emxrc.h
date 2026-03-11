#pragma once

#define EMXRC_MAX_VARS 255
#define EMXRC_MAX_EXECS 255

#define EMXRC_PATH "/emr/.emxrc"

typedef enum {
    EMXRC_FMT_ELF = 0,    // plain elf
    EMXRC_FMT_EP = 1,     // emex package
    EMXRC_FMT_INHERIT = 2,// if no -f just use the var's format (ep:, elf:)
} emxrc_fmt_t;

typedef struct {
    char name[64];
    char path[256];
    emxrc_fmt_t fmt;
} emxrc_var_t;

typedef struct {
    char var_name[64];
    emxrc_fmt_t fmt;
} emxrc_exec_t;

typedef struct {
    emxrc_var_t vars[EMXRC_MAX_VARS];
    int var_count;
    emxrc_exec_t execs[EMXRC_MAX_EXECS];
    int exec_count;
} emxrc_t;

int emxrc_parse(const char *path, emxrc_t *out);
void emxrc_run(emxrc_t *rc);
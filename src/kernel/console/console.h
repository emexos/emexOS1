#ifndef CONSOLE_H
#define CONSOLE_H

#include "../../../shared/types.h"

#define MAX_INPUT_LEN 256
#define MAX_CMDS 32

#pragma once

// function header macro for command declarations it should be easier to port it to future syscalls
#define FHDR(name) void name(const char* s)

typedef struct {
    void (*func)(const char*);
    const char *name;
    const char *description;
    const char *usage;
} console_cmd_t;

// for the 'help <command>'
#define CMDENTRY(func, name, desc, usage) { func, name, desc, usage }


void console_init(void);
void console_run(void);

void console_handle_key(char c);
void console_execute(const char *input);


void shell_clear_screen(u32 color);
void shell_print_prompt(void);
void shell_redraw_input(void);


console_cmd_t* console_find_cmd(const char *name);
FHDR(cmd_echo);
FHDR(cmd_clear);
FHDR(cmd_help);
FHDR(cmd_modules);
FHDR(cmd_meminfo);
FHDR(cmd_sysinfo);
FHDR(cmd_date);
FHDR(cmd_uptime);

#endif

#ifndef CONSOLE_H
#define CONSOLE_H

#include <kernel/graph/graphics.h>
#include <kernel/graph/fm.h>
#include <kernel/include/ports.h>
#include <kernel/communication/serial.h>
#include <drivers/ps2/keyboard/keyboard.h>
#include "graph/uno.h"
#include "graph/dos.h"
#include <kernel/graph/theme.h>
#include <kernel/graph/fm.h>
#include <theme/doccr.h>
#include <config/user.h>
#include <config/system.h>
#include "login/login.h"
#include <config/user_config.h>
#include <kernel/proc/scheduler.h>
#include <kernel/proc/proc_manager.h>

#include <string/string.h>

#include <kernel/module/module.h>
extern driver_module console_module;

#define MAX_INPUT_LEN 256
#define MAX_PATH_LEN 256
#define MAX_CMDS 32
#define MAX_CHAINED_CMDS 8

#define CONSOLE_APP_NAME "console"
#define CONSOLE_NAME "ekmsh" // emex-kernelmode-shell
#define WRONG_COMMAND_CL GFX_RED

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

extern char cwd[];
//for help.c
extern console_cmd_t commands[];
extern int cmd_count;

void console_init(void);
void console_run(void);

void console_handle_key(char c);
void console_handle_key_event(key_event_t *event);
void console_execute(const char *input);

void shell_clear_screen(u32 color);
void shell_print_prompt(void);
void shell_redraw_input(void);

void cursor_(void);
void cursor_draw(void);
void cursor_c(void);
void cursor_redraw(void);
void cursor_enable(void);
void cursor_disable(void);
void cursor_reset_blink(void);


void console_config_init(void); // config for promt in /.config/ekmsh/promts/promt.conf


console_cmd_t* console_find_cmd(const char *name);

int parse_color(const char *color_str, u32 *out_color);
void parse_and_execute_chained(const char *input);

#endif

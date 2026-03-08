#ifndef GEN_H
#define GEN_H

#include <kernel/file_systems/vfs/vfs.h>
#include_next <string/string.h>

#define CONSOLECONFIG "/.config/ekmsh"
#define PROMPT_PATH "/.config/ekmsh/prompt.cfg"

#define MAX_INPUT_LEN 256
#define MAX_PATH_LEN 256
#define MAX_CMDS 32
#define MAX_CHAINED_CMDS 8

#define CONSOLE_APP_NAME "console"
#define CONSOLE_WINDOW "w1"
#define CONSOLE_NAME "ekmsh" // emex-kernelmode-shell
#define WRONG_COMMAND_CL GFX_RED

extern char cwd[MAX_PATH_LEN];

void promptdirs(void);
void promptcont(void);
void console_init_gen(void);
void init_consoletheme(void);
void running_console_config_init(void);

#endif

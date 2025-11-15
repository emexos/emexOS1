#include "console.h"
#include "functions.h"

static char input_buffer[MAX_INPUT_LEN];
static int input_pos = 0;

//----------------------------------
// ! IMPORTANT FOR NEW COMMANDS !
static int cmd_count = 11;

static console_cmd_t commands[MAX_CMDS] = {
    CMDENTRY(cmd_echo, "echo", "prints text to console", "echo [text]"),
    CMDENTRY(cmd_clear, "clear", "clears the screen", "clear [color]"),
    CMDENTRY(cmd_help, "help", "displays all available commands", "help [command]"),
    CMDENTRY(cmd_fsize, "fsize", "change font size", "fsize [2-4]"),
    CMDENTRY(cmd_modules, "modules", "list loaded modules", "modules"),
    CMDENTRY(cmd_meminfo, "meminfo", "displays memory infos", "meminfo"),
    //CMDENTRY(cmd_memtest, "memtest", "Memory test suite", "memtest"),
    CMDENTRY(cmd_sysinfo, "dofetch", "displays doccrOS fetch", "dofetch"),
    CMDENTRY(cmd_cal, "calendar", "displays current date & time", "calendar"),
    CMDENTRY(cmd_date, "date", "displays current date", "date"),
    CMDENTRY(cmd_uptime, "uptime", "System uptime", "uptime"),
    CMDENTRY(cmd_time, "time", "displays current time", "time"),
};

//----------------------------------

//module---------------------------
static int console_module_init(void) {
    // console already initialized in main
    return 0;
}

static void console_module_fini(void) {
    //
}

driver_module console_module = (driver_module) {
    .name = "console",
    .mount = "/dev/console",
    .version = VERSION_NUM(0, 1, 1, 0), //should print like [v0.1.0.0]
    .init = console_module_init,
    .fini = console_module_fini,
    .open = NULL, // later for fs
    .read = NULL, // later for fs
    .write = NULL, // later for fs
};

//---------------------------------

void console_init(void)
{
    input_pos = 0;
    input_buffer[0] = '\0';

    clear(CONSOLESCREEN_COLOR);
    reset_cursor();
    //
    //module_register_driver(&console_module);

    if (cursor_x == 0 && cursor_y == 0) {
        clear(CONSOLESCREEN_COLOR);
        reset_cursor();
    }

    shell_print_prompt();
}

void console_run(void)
{
    // main console loop will be called from keyboard handler
    // this function exists for future expansion like syscalls maybe for a app like a code editor which needs a console
    // actually idk why i created this xd
}

void console_handle_key(char c)
{
    if (c == '\n') {
        // execute command when enter
        putchar('\n', GFX_WHITE);

        if (input_pos > 0) {
            input_buffer[input_pos] = '\0';
            console_execute(input_buffer);
            input_pos = 0;
            input_buffer[0] = '\0';
        }

        shell_print_prompt();
        return;
    }

    if (c == '\r') {
        putchar('\n', GFX_WHITE);
        input_buffer[input_pos++] = '\n';
        return;
    }

    if (c == '\b') {
        if (input_pos > 0) {
            input_pos--;
            input_buffer[input_pos] = '\0';

            // just move the cursor back then print space, draw rext, and move back again
            u32 char_width = 8 * font_scale + font_scale;
            if (cursor_x >= 20 + char_width) {
                cursor_x -= char_width;
                putchar(' ', GFX_WHITE);
                cursor_x -= char_width;

                draw_rect(cursor_x, cursor_y, char_width, 8 * font_scale, CONSOLESCREEN_COLOR);
            }
        }
        return;
    }

    // add character to buffer
    if (input_pos < MAX_INPUT_LEN - 1) {
        input_buffer[input_pos++] = c;
        input_buffer[input_pos] = '\0';
        putchar(c, GFX_WHITE);
    }
}

void console_execute(const char *input)
{
    //TODO:
    // OF UNKOWN BUGS THE SYSTEM CRASHES WHEN YOU ENTER A WRONG COMMAND
    // not anymore :)

    // skip leading spaces
    while (*input == ' ') input++;

    if (*input == '\0') return;

    // find command name end
    const char *end = input;
    while (*end && *end != ' ') end++;

    // extract it
    char cmd_name[64];
    int len = end - input;
    if (len >= 64) len = 63;

    for (int i = 0; i < len; i++) {
        cmd_name[i] = input[i];
    }
    cmd_name[len] = '\0';

    // find arguments
    const char *args = end;
    while (*args == ' ') args++;

    console_cmd_t *cmd = console_find_cmd(cmd_name);
    if (cmd) {
        cmd->func(args);
    } else {
        print("> Unknown command, Type 'help' for available commands...", GFX_RED);
    }
}

console_cmd_t* console_find_cmd(const char *name)
{
    for (int i = 0; i < cmd_count; i++) {
        // simple string comparison
        const char *a = name;
        const char *b = commands[i].name;
        int match = 1;

        while (*a && *b) {
            if (*a != *b) {
                match = 0;
                break;
            }
            a++;
            b++;
        }

        if (match && *a == '\0' && *b == '\0') {
            return &commands[i];
        }
    }
    return NULL;
}

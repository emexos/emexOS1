#include "console.h"
#include "functions.h"
#include "../../libs/print/print.h"
#include "../../libs/graphics/graphics.h"
//#include "../../libs/string/string.h"
#include "../../../shared/theme/doccr.h"

static char input_buffer[MAX_INPUT_LEN];
static int input_pos = 0;

//----------------------------------
// ! IMPORTANT FOR NEW COMMANDS !
static int cmd_count = 7;

static console_cmd_t commands[MAX_CMDS] = {
    CMDENTRY(cmd_echo, "echo", "prints text to console", "echo [text]"),
    CMDENTRY(cmd_clear, "clear", "clears the screen", "clear [color]"),
    CMDENTRY(cmd_help, "help", "displays all available commands", "help [command]"),
    CMDENTRY(cmd_modules, "modules", "list loaded modules", "modules"),
    CMDENTRY(cmd_meminfo, "meminfo", "displays memory infos", "meminfo"),
    //CMDENTRY(cmd_memtest, "memtest", "Memory test suite", "memtest"),
    CMDENTRY(cmd_sysinfo, "dofetch", "displays doccrOS fetch", "dofetch"),
    CMDENTRY(cmd_date, "date", "displays the date", "date"),
    //CMDENTRY(cmd_uptime, "uptime", "System uptime", "uptime")
};

//----------------------------------

void console_init(void)
{
    input_pos = 0;
    input_buffer[0] = '\0';

    //clear(CONSOLESCREEN_COLOR);
    //reset_cursor();

    if (cursor_x == 10 && cursor_y == 10) {
        clear(CONSOLESCREEN_COLOR);
        reset_cursor();
        print(" ", GFX_WHITE); //there is a glitch where the first print is always 10px left printed... so we just print the first line with nothing
                                //print("console", GFX_WHITE);
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

    if (c == '\b') {
        if (input_pos > 0) {
            input_pos--;
            input_buffer[input_pos] = '\0';

            // just move the cursor back then print space, draw rext, and move back again
            if (cursor_x >= 29) {
                cursor_x -= 9;
                putchar(' ', GFX_WHITE);
                cursor_x -= 9;

                draw_rect(cursor_x, cursor_y, 9, 16, CONSOLESCREEN_COLOR);
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
        print("Unknown command, Type 'help' for available commands...\n", GFX_RED);
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

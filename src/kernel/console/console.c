#include "console.h"
#include "functions.h"

static char input_buffer[MAX_INPUT_LEN];
static int input_pos = 0;
char cwd[MAX_PATH_LEN] = "/";
//int is_login_active = 0;

//----------------------------------
// ! IMPORTANT FOR NEW COMMANDS !
int cmd_count = 26;

console_cmd_t commands[MAX_CMDS] = {
    CMDENTRY(cmd_echo, "echo", "prints text to console", "echo [text]"),
    CMDENTRY(cmd_clear, "clear", "clears the screen", "clear [color]"),
    CMDENTRY(cmd_help, "help", "displays all available commands", "help [command]"),
    CMDENTRY(cmd_fsize, "scale", "change screen size", "scale [2-4]"),
    CMDENTRY(cmd_modules, "modules", "list loaded modules", "modules"),
    CMDENTRY(cmd_meminfo, "meminfo", "displays memory infos", "meminfo"),
    //CMDENTRY(cmd_memtest, "memtest", "Memory test suite", "memtest"),
    CMDENTRY(cmd_sysinfo, "dofetch", "displays doccrOS fetch", "dofetch"),
    CMDENTRY(cmd_cal, "calendar", "displays current date & time", "calendar"),
    CMDENTRY(cmd_date, "date", "displays current date", "date"),
    CMDENTRY(cmd_uptime, "uptime", "System uptime", "uptime"),
    CMDENTRY(cmd_time, "time", "displays current time", "time"),
    CMDENTRY(cmd_poweroff, "poweroff", "Power off the system", "poweroff"),
    CMDENTRY(cmd_reboot, "reboot", "Reboot the system", "reboot"),
    CMDENTRY(cmd_shutdown, "shutdown", "Shutdown the system", "shutdown"),
    CMDENTRY(cmd_cat, "cat", "show file", "cat <file>"),
    CMDENTRY(cmd_ls, "ls", "list directory contents", "ls [path]"),
    CMDENTRY(cmd_cd, "cd", "Change directory", "cd [path]"),
    CMDENTRY(cmd_tree, "tree", "shows every folder, file * content", "tree"),
    CMDENTRY(cmd_mkdir, "mkdir", "create directory", "mkdir <path>"),
    CMDENTRY(cmd_font, "font", "change console font", "font [0-1]"),
    CMDENTRY(cmd_keymap, "keymap", "change keyboard layout", "keymap [US|DE]"),
    CMDENTRY(cmd_whoami, "whoami", "display current user", "whoami"),
    CMDENTRY(cmd_source, "source", "reload configuration", "source console"),
    CMDENTRY(cmd_touch, "touch", "create empty file", "touch <file>"),
    CMDENTRY(cmd_view, "view", "view BMP image", "view <image.bmp>"),
    //CMDENTRY(cmd_edit, "edit", "simple text editor", "edit <file>"),
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
    .version = VERSION_NUM(0, 1, 2, 0), //should print like [v0.1.0.0]
    .init = console_module_init,
    .fini = console_module_fini,
    .open = NULL, // later for fs
    .read = NULL, // later for fs
    .write = NULL, // later for fs
};

//---------------------------------


//extern void prompt_config_init();
void console_init(void)
{
    input_pos = 0;
    input_buffer[0] = '\0';
    char buf[32];

    print("[CONSOLE] ", GFX_GRAY_70);
    print("starting console...\n", white());

    //sconsole_theme(THEME_FLU);
    f_setcontext(FONT_8X8);
    clear(CONSOLESCREEN_BG_COLOR);

    print("[FM] ", GFX_GRAY_70);
    print("scaling font...\n", white());
    font_scale = 2;
    str_append_uint(buf, font_scale);
    print("[FM] ", GFX_GRAY_70);
    print("font scaled to: ", white());
    print(buf, white());
    print("\n", white());

    buf[0] = '\0'; //reset
    //reset_cursor();
    /*
    //module_register_driver(&console_module);

    if (cursor_x == 0 && cursor_y == 0) {
        clear(CONSOLESCREEN_COLOR);
        reset_cursor();
    }
    */

    cursor_x = 0;
    cursor_y = 0;

    //is_login_active = 1;
    //if (!login_authenticate()) {
        // Login failed - should never reach here due to lock
        //panic("Login authentication failed");
        //}
    //is_login_active = 0;


    //create config files for console:
    /*fs_mkdir("/.config/ekmsh");
    fs_mkdir("/.config/ekmsh/promts");
    fs_open("/.config/ekmsh/promts/promt.conf", O_CREAT | O_WRONLY);*/

    prompt_config_init();

    clear(CONSOLESCREEN_BG_COLOR);
    banner_init();

    // Initialize console window
    console_window_init();
    cursor_();

    shell_print_prompt();

    //console_execute("view /images/logo.bmp");
    cursor_draw();

    console_run();
}

void console_run(void)
{
    // main console loop will be called from keyboard handler
    // this function exists for future expansion like syscalls maybe for a app like a code editor which needs a console
    // actually idk why i created this xd
    while (1) {
        if (keyboard_has_key()) {
            key_event_t event;
            if (keyboard_get_event(&event)) {
                if (event.pressed) {
                    console_handle_key_event(&event);
                }
            }
        }
        // Let CPU rest
        __asm__ volatile("hlt");
    }
}

void console_handle_key_event(key_event_t *event) {
    cursor_c();

    // Handle Ctrl+S (save in edit mode)
    if ((event->modifiers & KEY_CTRL_MASK) && (event->keycode == 's' || event->keycode == 'S')) {
        // This will be handled by edit command
        return;
    }

    char c = (char)(event->keycode & 0xFF);
    console_handle_key(c);

    cursor_reset_blink();
    cursor_draw();
}

void console_handle_key(char c)
{
    cursor_c();
    if (c == '\n') {
        // execute command when enter
        putchar('\n', GFX_WHITE);

        if (input_pos > 0) {
            input_buffer[input_pos] = '\0';

            // check for && and use chained execution
            int has_chain = 0;
            for (int i = 0; i < input_pos - 1; i++) {
                if (input_buffer[i] == '&' && input_buffer[i+1] == '&') {
                    has_chain = 1;
                    break;
                }
            }

            // prints every command to the console
            printf("%s\n", input_buffer);

            if (has_chain) {
                parse_and_execute_chained(input_buffer);
            } else {
                console_execute(input_buffer);
            }


            input_pos = 0;
            input_buffer[0] = '\0';
        }

        cursor_reset_blink();
        //cursor_draw();
        shell_print_prompt();
        return;
    }

    if (c == '\r') {
        putchar('\n', GFX_WHITE);
        input_buffer[input_pos++] = '\n';
        cursor_draw();
        return;
    }

    if (c == '\b') {
        if (input_pos > 0) {
            input_pos--;
            input_buffer[input_pos] = '\0';

            // just move the cursor back then print space, draw rext, and move back again
            u32 char_width = 8 * font_scale;
            if (cursor_x >= char_width) {
                cursor_x -= char_width;
                putchar(' ', GFX_WHITE);
                cursor_x -= char_width;

                draw_rect(cursor_x, cursor_y, char_width, 8 * font_scale, CONSOLESCREEN_BG_COLOR);
            }
        }
        cursor_reset_blink();
        cursor_draw();
        return;
    }

    console_window_check_scroll();

    // add character to buffer
    if (input_pos < MAX_INPUT_LEN - 1) {
        input_buffer[input_pos++] = c;
        input_buffer[input_pos] = '\0';
        putchar(c, GFX_WHITE);
    }
    cursor_reset_blink();
    cursor_draw();
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

        banner_force_update();

        // for PC_NAME and USER_NAME
        user_config_reload();
    } else {
        print(CONSOLE_NAME, WRONG_COMMAND_CL);
        print(": command not found:", WRONG_COMMAND_CL);
        print(cmd_name, WRONG_COMMAND_CL);
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

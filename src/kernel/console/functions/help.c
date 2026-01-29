#include <kernel/console/console.h>

#define GEN "[GENERIC]"
#define FS "[FILESYSTEM]"
#define GUI "[GRAPHICS]"
#define PADDING 12

FHDR(cmd_help)
{
    extern console_cmd_t commands[];
    extern int cmd_count;

    if (*s == '\0') {
        // these are COMMON commands, soon i will split all comands
        print("[COMMON]\n", GFX_YELLOW);

        for (int i = 0; i < cmd_count; i++) {
            char buf[128];
            str_copy(buf, "  ");
            str_append(buf, commands[i].name);

            int name_len = str_len(commands[i].name);
            int padding = PADDING - name_len;
            for (int p = 0; p < padding && p < 14; p++) {
                str_append(buf, " ");
            }

            str_append(buf, "- ");
            str_append(buf, commands[i].description);
            str_append(buf, "\n");
            print(buf, GFX_WHITE);
        }

        print("Type 'help <command>' for details", GFX_GRAY_50);
    } else {
        // specific commands
        const char *p = s;
        while (*p == ' ') p++;

        console_cmd_t *cmd = console_find_cmd(p);
        if (cmd) {
            char buf[128];

            str_copy(buf, "\n");
            str_append(buf, cmd->description);
            print(buf, GFX_WHITE);

            str_copy(buf, "\nUsage: ");
            str_append(buf, cmd->usage);
            print(buf, GFX_YELLOW);
            print("\n", GFX_WHITE);
        } else {
            print("\nCommand not found", GFX_RED);
        }
    }
}

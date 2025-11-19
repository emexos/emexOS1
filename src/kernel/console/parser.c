#include <kernel/console/console.h>

// parse color argument if provided
int parse_color(const char *color_str, u32 *out_color) {
    if (!color_str || !out_color) return 0;

    const char *p = color_str;
    while (*p == ' ') p++;

    // simple color name parsing
    if (*p == 'b' && *(p+1) == 'l' && *(p+2) == 'a' && *(p+3) == 'c' && *(p+4) == 'k' && *(p+5) == '\0') {
        *out_color = GFX_BLACK;
        return 1;
    } else if (*p == 'w' && *(p+1) == 'h' && *(p+2) == 'i' && *(p+3) == 't' && *(p+4) == 'e' && *(p+5) == '\0') {
        *out_color = GFX_WHITE;
        return 1;
    } else if (*p == 'r' && *(p+1) == 'e' && *(p+2) == 'd' && *(p+3) == '\0') {
        *out_color = GFX_RED;
        return 1;
    } else if (*p == 'g' && *(p+1) == 'r' && *(p+2) == 'e' && *(p+3) == 'e' && *(p+4) == 'n' && *(p+5) == '\0') {
        *out_color = GFX_GREEN;
        return 1;
    } else if (*p == 'b' && *(p+1) == 'l' && *(p+2) == 'u' && *(p+3) == 'e' && *(p+4) == '\0') {
        *out_color = GFX_BLUE;
        return 1;
    } else if (*p == 'c' && *(p+1) == 'y' && *(p+2) == 'a' && *(p+3) == 'n' && *(p+4) == '\0') {
        *out_color = GFX_CYAN;
        return 1;
    } else if (*p == 'y' && *(p+1) == 'e' && *(p+2) == 'l' && *(p+3) == 'l' && *(p+4) == 'o' && *(p+5) == 'w' && *(p+6) == '\0') {
        *out_color = GFX_YELLOW;
        return 1;
    } else if (*p == 'p' && *(p+1) == 'u' && *(p+2) == 'r' && *(p+3) == 'p' && *(p+4) == 'l' && *(p+5) == 'e' && *(p+6) == '\0') {
        *out_color = GFX_PURPLE;
        return 1;
    }

    return 0;
}

// split input in && then execute the commands
void parse_and_execute_chained(const char *input) {
    if (!input || *input == '\0') return;

    char commands[MAX_CHAINED_CMDS][MAX_INPUT_LEN];
    int cmd_count = 0;
    int cmd_pos = 0;

    const char *p = input;

    // split input in &&
    while (*p && cmd_count < MAX_CHAINED_CMDS) {
        while (*p == ' ') p++; // space after/ before &&

        // copy command until && or end
        cmd_pos = 0;
        while (*p && cmd_pos < MAX_INPUT_LEN - 1) {
            if (*p == '&' && *(p+1) == '&') { // == &&
                commands[cmd_count][cmd_pos] = '\0';
                cmd_count++;
                p += 2;
                break;
            }
            commands[cmd_count][cmd_pos++] = *p++;
        }

        // if last command is or buffer is full
        if (*p == '\0' || cmd_pos >= MAX_INPUT_LEN - 1) {
            commands[cmd_count][cmd_pos] = '\0';
            cmd_count++;
            break;
        }
    }

    // execute all commands
    for (int i = 0; i < cmd_count; i++) {
        //trailing
        int len = 0;
        while (commands[i][len]) len++;
        len--;
        while (len >= 0 && commands[i][len] == ' ') {
            commands[i][len] = '\0';
            len--;
        }

        if (commands[i][0] != '\0') {
            console_execute(commands[i]);
        }
    }
}

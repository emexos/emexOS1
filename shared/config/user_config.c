#include "user_config.h"
#include <kernel/file_systems/vfs/vfs.h>
#include <string/string.h>
#include <memory/main.h>

static user_config_t current_config = {
    .pc_name = PC_NAME,
    .user_name = USER_NAME,
    .password = DEFAULT_PWD,
    .keymap = DEFAULT_KM
};

void uci(void) {
    // create /emr/config directory
    log("[UCI]", "initialize UCI... ", d);
    // writes the default conf to vfs
    uci_save();
    const char* pc_name = uci_get_pc_name();
    const char* user_name = uci_get_user_name();
    const char* password = uci_get_password();
    const char* keymap = uci_get_keymap();


    log("\n     ", pc_name, d);
    log("\n     ", user_name, d);
    log("\n     ", password, d);
    log("\n     ", keymap, d);

    log("\n[UCI]", "save UCI... \n", d);
}

int uci_load_users_ini(void) {
    int fd = fs_open(USERINI_PATH "", O_RDONLY); // idk how this can work...
    if (fd < 0) return -1;

    char buf[1024];
    ssize_t bytes_read = fs_read(fd, buf, sizeof(buf) - 1);
    fs_close(fd);

    if (bytes_read <= 0) return -1;
    buf[bytes_read] = '\0';

    char *line = buf;
    char current_section[32] = {0};
    int in_section = 0;

    while (*line) {
        while (*line == ' ' || *line == '\t') line++;

        // this checks the section []
        // it loads the username between []
        if (*line == '[') {
            line++; // Skip '['
            char *end = line;
            while (*end && *end != ']') end++;
            int len = end - line;
            if (len > 0 && len < sizeof(current_section)) {
                for (int i = 0; i < len; i++) {
                    current_section[i] = line[i];
                }
                current_section[len] = '\0';
            }
            in_section = 1;
            line = (*end) ? end + 1 : end;
            continue;
        }

        // it needs to have permissions of user/guest
        if (in_section && str_starts_with(line, "permissions=")) {
            line += 12; // skip "permissions="
            char *end = line;
            while (*end && *end != '\n') end++;
            int len = end - line;

            char perm[16] = {0};
            if (len > 0 && len < sizeof(perm)) {
                for (int i = 0; i < len; i++) {
                    perm[i] = line[i];
                }
                perm[len] = '\0';
            }

            // theres also administrator but i don't want that rn...
            // also this is just a simple implementation withouth password hashing
            if (str_equals(perm, "user") || str_equals(perm, "guest")) {
                // in future "guest" will only has less permissions than "user"
                // make current_config.user_name to the section name
                str_copy(current_config.user_name, current_section);
            }

            line = (*end) ? end + 1 : end;
            continue;
        }
        while (*line && *line != '\n') line++;
        if (*line == '\n') line++;
    }
    return 0;
}

int uci_save(void) {
    // ensure we load the correct username from users.ini
    uci_load_users_ini();

    int fd = fs_open("/emr/config/user.emcg", O_CREAT | O_WRONLY);
    if (fd < 0) return -1;

    char buf[256];

    // Write PC_NAME
    str_copy(buf, "PC_NAME: ");
    str_append(buf, current_config.pc_name);
    str_append(buf, "\n");
    fs_write(fd, buf, str_len(buf));

    // Write USER_NAME
    str_copy(buf, "USER_NAME: ");
    str_append(buf, current_config.user_name);
    str_append(buf, "\n");
    fs_write(fd, buf, str_len(buf));

    // Write PASSWORD
    str_copy(buf, "PASSWORD: ");
    str_append(buf, current_config.password);
    str_append(buf, "\n");
    fs_write(fd, buf, str_len(buf));

    // Write KM
    str_copy(buf, "KM: ");
    str_append(buf, current_config.keymap);
    str_append(buf, "\n");
    fs_write(fd, buf, str_len(buf));

    fs_close(fd);
    return 0;
}

int uci_load(void) {
    int fd = fs_open("/emr/config/user.emcg", O_RDONLY);
    if (fd < 0) return -1;

    char buf[256];
    ssize_t bytes_read = fs_read(fd, buf, sizeof(buf) - 1);
    fs_close(fd);

    if (bytes_read <= 0) return -1;
    buf[bytes_read] = '\0';

    // Parse config file
    char *line = buf;
    while (*line) {
        // Skip whitespace
        while (*line == ' ' || *line == '\t') line++;

        if (str_starts_with(line, "PC_NAME: ")) {
            line += 9; // Skip "PC_NAME="
            char *end = line;
            while (*end && *end != '\n') end++;
            int len = end - line;
            if (len > 0 && len < sizeof(current_config.pc_name)) {
                for (int i = 0; i < len; i++) {
                    current_config.pc_name[i] = line[i];
                }
                current_config.pc_name[len] = '\0';
            }
            line = (*end) ? end + 1 : end;
        }
        else if (str_starts_with(line, "USER_NAME: ")) {
            line += 11; // Skip "USER_NAME="
            char *end = line;
            while (*end && *end != '\n') end++;
            int len = end - line;
            if (len > 0 && len < sizeof(current_config.user_name)) {
                for (int i = 0; i < len; i++) {
                    current_config.user_name[i] = line[i];
                }
                current_config.user_name[len] = '\0';
            }
            line = (*end) ? end + 1 : end;
        }
        else if (str_starts_with(line, "PASSWORD: ")) {
            line += 10; // Skip "PASSWORD="
            char *end = line;
            while (*end && *end != '\n') end++;
            int len = end - line;
            if (len > 0 && len < sizeof(current_config.password)) {
                for (int i = 0; i < len; i++) {
                    current_config.password[i] = line[i];
                }
                current_config.password[len] = '\0';
            }
            line = (*end) ? end + 1 : end;
        }
        else if (str_starts_with(line, "KM: ")) {
            line += 4; // Skip "KM: "
            char *end = line;
            while (*end && *end != '\n') end++;
            int len = end - line;
            if (len > 0 && len < sizeof(current_config.keymap)) {
                for (int i = 0; i < len; i++) {
                    current_config.keymap[i] = line[i];
                }
                current_config.keymap[len] = '\0';
            }
            line = (*end) ? end + 1 : end;
        }
        else {
            // Skip to next line
            while (*line && *line != '\n') line++;
            if (*line == '\n') line++;
        }
    }

    return 0;
}

const char* uci_get_pc_name(void) {
    return current_config.pc_name;
}

const char* uci_get_user_name(void) {
    return current_config.user_name;
}

const char* uci_get_password(void) {
    return current_config.password;
}

const char* uci_get_keymap(void) {
    return current_config.keymap;
}

void uci_reload(void) {
    uci_load();
}

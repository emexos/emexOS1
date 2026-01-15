#include "user_config.h"
#include <kernel/file_systems/vfs/vfs.h>
#include <string/string.h>
#include <memory/main.h>

static user_config_t current_config = {
    .pc_name = PC_NAME,
    .user_name = USER_NAME,
    .password = DEFAULT_PWD
};

void user_config_init(void) {
    // Create /tmp/config directory

    // writes the default conf to vfs
    user_config_save();
}

int user_config_save(void) {
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

    fs_close(fd);
    return 0;
}

int user_config_load(void) {
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
        else {
            // Skip to next line
            while (*line && *line != '\n') line++;
            if (*line == '\n') line++;
        }
    }

    return 0;
}

const char* user_config_get_pc_name(void) {
    return current_config.pc_name;
}

const char* user_config_get_user_name(void) {
    return current_config.user_name;
}

const char* user_config_get_password(void) {
    return current_config.password;
}

void user_config_reload(void) {
    user_config_load();
}

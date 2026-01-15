#ifndef USER_CONFIG_H
#define USER_CONFIG_H

#include <types.h>
#include <config/user.h>

typedef struct {
    char pc_name[64];
    char user_name[64];
    char password[64];
} user_config_t;

// Initialize and save default config to VFS
void user_config_init(void);

// Save current config to VFS
int user_config_save(void);

// Load config from VFS
int user_config_load(void);

// Getters for current config
const char* user_config_get_pc_name(void);
const char* user_config_get_user_name(void);
const char* user_config_get_password(void);

// Reload config (for use after file changes)
void user_config_reload(void);

#endif

#ifndef USERS_H

#define USERS_H

#include <types.h>

#define USERS_MAX      32
#define USER_NAME_MAX  64
#define USER_HOME_MAX 128

#define UID_ROOT 		0
#define GID_ROOT 		0

typedef struct
{
    char 	username[USER_NAME_MAX];
    u32  	uid;
    u32  	gid;
    char 	home[USER_HOME_MAX];
    u8   	valid;
} user_entry_t;

// vfs permisssion chec
extern u32 g_current_uid;
extern u32 g_current_gid;

void users_init(void);
void users_setup_homes(void);

int users_load(const char *path);

u32 users_get_count(void);

user_entry_t *users_get_by_uid(u32 uid);
user_entry_t *users_get_by_name(const char *name);
user_entry_t *users_get_first_regular(void);

#endif
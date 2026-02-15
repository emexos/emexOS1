#ifndef CONF_H
#define CONF_H

#define CONF_MAX_KEY 64
#define CONF_MAX_VALUE 256

typedef struct {
    char key[CONF_MAX_KEY];
    char value[CONF_MAX_VALUE];
} conf_entry_t;

int conf_load(const char *path, conf_entry_t *entries, int max_entries);
const char* conf_get(conf_entry_t *entries, int count, const char *key);

#endif

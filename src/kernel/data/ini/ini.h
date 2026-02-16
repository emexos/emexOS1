#ifndef INI_H
#define INI_H

#include <string/string.h>
#include <types.h>

typedef struct {
    char *section;
    char *key;
    char *value;
} ini_entry_t;

void ini_parse(char *buf, ini_entry_t *entries, int *entry_count, int max_entries);
char* ini_get(ini_entry_t *entries, int entry_count, const char *section, const char *key);

#endif

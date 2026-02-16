#include "ini.h"

// just for users.ini and system.ini

static void skipWS(char **p) {
    while (**p == ' ' || **p == '\t' || **p == '\r' || **p == '\n') (*p)++;
}

void ini_parse(char *buf, ini_entry_t *entries, int *entry_count, int max_entries) {
    char *p = buf;
    char *cur_section = NULL;
    int count = 0;

    skipWS(&p);

    while (*p && count < max_entries) {
        if (*p == ';' || *p == '#') {
            while (*p && *p != '\n') p++;
            skipWS(&p);
            continue;
        }

        if (*p == '[') {
            p++;
            char *start = p;
            while (*p && *p != ']') p++;
            if (*p) { *p = '\0'; p++; }
            cur_section = start;
            skipWS(&p);
            continue;
        }

        char *key_start = p;
        while (*p && *p != '=' && *p != '\n') p++;
        if (*p != '=') { while (*p && *p != '\n') p++; skipWS(&p); continue; }
        *p = '\0';
        char *value_start = ++p;

        while (*p && *p != '\n') p++;
        if (*p) { *p = '\0'; p++; }

        entries[count].section = cur_section;
        entries[count].key = key_start;
        entries[count].value = value_start;
        count++;

        skipWS(&p);
    }

    *entry_count = count;
}

char* ini_get(ini_entry_t *entries, int entry_count, const char *section, const char *key) {
    for (int i = 0; i < entry_count; i++) {
        if ((entries[i].section == NULL && section == NULL) || str_equals(entries[i].section, section)) {
            if (str_equals(entries[i].key, key)) return entries[i].value;
        }
    }
    return NULL;
}

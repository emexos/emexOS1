#ifndef JSON_H
#define JSON_H

#include <kernel/file_systems/vfs/vfs.h>
#include <string/string.h>

//NOTE:
// this is a flat JSON parser with comments cuz i hate that default json doesnt have that.....

// i should prob make 0 limits just like the html parser but nah...
#define JSON_MAX_KEY 128
#define JSON_MAX_VALUE 256

typedef struct {
    char key[JSON_MAX_KEY];
    char value[JSON_MAX_VALUE];
} json_entry_t;

// loads flat JSON into entries array
int json_load(const char *path, json_entry_t *entries, int max_entries);
const char* json_get(json_entry_t *entries, int count, const char *key);

#endif

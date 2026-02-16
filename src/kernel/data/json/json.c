#include "json.h"

// helpers
// skip spaces, tabs, newlines...
static void skipWS1(const char **p) {
    while (**p == ' ' || **p == '\t' || **p == '\n' || **p == '\r') (*p)++;
}
// skip comments like // or /**/
static void skipCOM1(const char **p)
{
    while (**p) {
        if ((*p)[0] == '/' && (*p)[1] == '/') { // line comment
            *p += 2;
            while (**p && **p != '\n') (*p)++;
        } else if ((*p)[0] == '/' && (*p)[1] == '*') { // block comment
            *p += 2;
            while (**p && !((*p)[0] == '*' && (*p)[1] == '/')) (*p)++;
            if (**p) *p += 2;
        } else break;
        skipWS1(p);
    }
}
// read string inside ""
static int read_string(const char **p, char *buffer, int max_len)
{
    if (**p != '\"') return -1;
    (*p)++;
    int i = 0;
    while (**p && **p != '\"' && i < max_len - 1) {
        buffer[i++] = **p;
        (*p)++;
    }
    buffer[i] = '\0';
    if (**p == '\"') (*p)++;
    return 0;
}
// combines parentkeys and childkeys like parent.child
static void append_key(const char *parent, const char *child, char *out) {
    if (parent[0] == '\0') {
        str_copy(out, child);
    } else {
        str_copy(out, parent);
        str_append(out, ".");
        str_append(out, child);
    }
}

// parser
// forward declaration for recursion
static int parse_object(const char **p, const char *prefix, json_entry_t *entries, int max_entries, int count);

// parse an array like [ {...}, {...} ]
static int parse_array(const char **p, const char *prefix, json_entry_t *entries, int max_entries, int count) {
    if (**p != '[') return count;
    (*p)++;
    skipWS1(p);
    skipCOM1(p);

    int idx = 0;
    while (**p && **p != ']') {
        // make a key prefix with index like parent[0]
        char tmp[JSON_MAX_KEY];
        str_copy(tmp, prefix);

        char index_str[16];
        int len = 0;
        index_str[len++] = '[';
        int t = idx++;
        if (t >= 10) { index_str[len++] = '0' + t / 10; t %= 10; }
        index_str[len++] = '0' + t;
        index_str[len++] = ']';
        index_str[len] = '\0';

        str_append(tmp, index_str);

        skipWS1(p);
        skipCOM1(p);

        if (**p == '{') {
            count = parse_object(p, tmp, entries, max_entries, count);
        } else {
            char value[JSON_MAX_VALUE];
            if (read_string(p, value, JSON_MAX_VALUE) == 0) {
                if (count < max_entries) {
                    str_copy(entries[count].key, tmp);
                    str_copy(entries[count].value, value);
                    count++;
                }
            }
        }

        skipWS1(p);
        skipCOM1(p);

        if (**p == ',') (*p)++;
        skipWS1(p);
        skipCOM1(p);
    }

    if (**p == ']') (*p)++;
    return count;
}

// parse an object like { "key": "value", ... }
static int parse_object(const char **p, const char *prefix, json_entry_t *entries, int max_entries, int count) {
    if (**p != '{') return count;
    (*p)++;
    skipWS1(p);
    skipCOM1(p);

    while (**p && **p != '}') {
        char key[JSON_MAX_KEY];
        if (read_string(p, key, JSON_MAX_KEY) != 0) break;

        skipWS1(p);
        skipCOM1(p);

        if (**p != ':') break;
        (*p)++;
        skipWS1(p);
        skipCOM1(p);

        char new_prefix[JSON_MAX_KEY];
        append_key(prefix, key, new_prefix);

        if (**p == '{') {
            count = parse_object(p, new_prefix, entries, max_entries, count);
        } else if (**p == '[') {
            count = parse_array(p, new_prefix, entries, max_entries, count);
        } else {
            char value[JSON_MAX_VALUE];
            if (read_string(p, value, JSON_MAX_VALUE) == 0) {
                if (count < max_entries) {
                    str_copy(entries[count].key, new_prefix);
                    str_copy(entries[count].value, value);
                    count++;
                }
            }
        }

        skipWS1(p);
        skipCOM1(p);

        if (**p == ',') (*p)++;
        skipWS1(p);
        skipCOM1(p);
    }

    if (**p == '}') (*p)++;
    return count;
}

// pub ---

int json_load(const char *path, json_entry_t *entries, int max_entries) {
    int fd = fs_open(path, O_RDONLY);
    if (fd < 0) return -1;

    char buf[8192];
    ssize_t bytes = fs_read(fd, buf, sizeof(buf) - 1);
    fs_close(fd);
    if (bytes <= 0) return -1;
    buf[bytes] = '\0';

    const char *p = buf;
    skipWS1(&p);
    skipCOM1(&p);

    int count = 0;
    char empty_prefix[1] = "";
    if (*p == '{') {
        count = parse_object(&p, empty_prefix, entries, max_entries, count);
    }

    return count;
}

const char* json_get(json_entry_t *entries, int count, const char *key) {
    for (int i = 0; i < count; i++) {
        if (str_equals(entries[i].key, key)) return entries[i].value;
    }
    return NULL;
}

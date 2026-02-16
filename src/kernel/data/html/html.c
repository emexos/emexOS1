#include "html.h"

html_entry_t html_table[128];
int html_table_count = 0;

static void skipWS(char **p) {
    while (**p == ' ' || **p == '\t' || **p == '\r' || **p == '\n') (*p)++;
}

void htmlparse(char *buf) {
    char *p = buf;
    skipWS(&p);
    html_table_count = 0;

    while (*p) {
        if (*p == '<') {
            p++;
            if (*p == '/') {
                while (*p && *p != '>') p++;
                if (*p) p++;
                skipWS(&p);
                continue;
            }

            char *tag_start = p;
            while (*p && *p != '>' && *p != ' ' && *p != '\n') p++;
            char saved = *p;
            *p = '\0';
            char *tag_name = tag_start;
            *p = saved;
            while (*p && *p != '>') p++;
            if (*p) p++;

            char *content_start = p;
            while (*p && !(*p == '<' && *(p+1) == '/')) p++;
            char *content_end = p;
            if (content_end > content_start) *content_end = '\0';

            html_table[html_table_count].tag = tag_name;
            html_table[html_table_count].content = content_start;
            html_table_count++;

            if (*p == '<') {
                p += 2;
                while (*p && *p != '>') p++;
                if (*p) p++;
            }

            skipWS(&p);
        } else {
            p++;
        }
    }
}

char* htmlget(const char *tag) {
    for (int i = 0; i < html_table_count; i++) {
        if (str_equals(html_table[i].tag, tag)) return html_table[i].content;
    }
    return 0;
}

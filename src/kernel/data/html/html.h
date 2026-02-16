#ifndef HTML_H
#define HTML_H

#include "table.h"

typedef struct {
    char *tag;
    char *content;
} html_entry_t;

extern html_entry_t html_table[];
extern int html_table_count;

void htmlparse(char *buf);
char* htmlget(const char *tag);

#endif

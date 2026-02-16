#ifndef TABLE_H
#define TABLE_H

#include <types.h>
#include <string/string.h>

// list of available tags
static const char *html_tags[] = {
    "html",
    "head",
    "title",
    "body",
    "h1",
    "h2",
    "h3",
    "p",
    "div",
    "span",
    "a",
    "ul",
    "ol",
    "li",
    "table",
    "tr",
    "td",
    "th",
    "br",
    "hr"
};

static const int html_tags_count = sizeof(html_tags) / sizeof(html_tags[0]);

#endif

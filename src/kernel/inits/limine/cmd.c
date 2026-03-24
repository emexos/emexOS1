#include <kernel/include/reqs.h>
#include <string/string.h>

static char token[64] = {0};

void cmd_init(void) {
    if (!cmdline_request.response) return;
    const char *cl = cmdline_request.response->cmdline;
    if (!cl) return;

    int i = 0;
    while (cl[i] && i < 63) { token[i] = cl[i]; i++; }
    token[i] = '\0';
    printf("[CMD] cmdline: '%s'\n", token);
}

int cmd_is(const char *val){return str_equals(token,val);}

int cmd_has(const char *t){return str_contains(token,t);}
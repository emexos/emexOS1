#include "glime.h"

int gworkspace_init(glime_t *glime, u8 *name, u64 pos) {
    if (!glime) return 1;
    if (glime->workspaces_len > glime->workspaces_total) return 1;

    gworkspace_t *workspace = (gworkspace_t *) glime_create(glime, sizeof(gworkspace_t));
    if (!workspace) return 1;

    int seek = 0;
    while (name[seek] != '\0' && seek < GWORKSPACE_NAME_MAX - 1) {
        workspace->name[seek] = name[seek];
        seek++;
    }
    workspace->name[seek] = '\0';
    
    workspace->pos_x = 0;

    //@TODO: add handling a y moving
    workspace->pos_y = pos;
    workspace->gap_x = 10;
    workspace->sessions_total = GSESSION_LEN_MAX;
    workspace->session_active = 0;

    workspace->sessions = (gsession_t **) glime_alloc(glime, sizeof(gsession_t *), GSESSION_LEN_MAX); 
    if (!workspace->sessions) {
        return 1;
    }

    workspace->glime = glime;
    glime->workspaces[glime->workspaces_len] = workspace;
    glime->workspace_active = glime->workspaces_len;
    glime->workspaces_len++;

    return 0;
}

gworkspace_t *gworkspace_get_name(glime_t *glime, u8 *workspace_name) {
    if (!glime) return NULL;

    for (u64 i = 0; i < glime->workspaces_total; i++) {
        gworkspace_t *ws = glime->workspaces[i];
        u8 *wsname = ws->name;
        int seek = 0;
        int eq = 1;
        while (workspace_name[seek] != '\0') {
            if (wsname[seek] == '\0') {
                eq = 0;
                break;
            }
            if (wsname[seek] != workspace_name[seek]) {
                eq = 0;
                break;
            }
            seek += 1;
        }

        if (eq) return ws;
    }

    return NULL;
}

gworkspace_t *gworkspace_get_pos(glime_t *glime, u64 pos) {
    if (!glime) return NULL;

    for (u64 i = 0; i < glime->workspaces_total; i++) {
        gworkspace_t *ws = glime->workspaces[i];
        if (ws->pos_y == pos) return ws;
    }

    return NULL;
}

int gworkspace_posx_get(gworkspace_t *gworkspace, u64 *out) {
    if (!gworkspace) return 1;

    if (gworkspace->sessions_len == 0) {
        *out = 0;
        return 0;
    }

    u64 list_len = gworkspace->sessions_len;
    u64 ret = 0;

    for (u64 i = 0; i < list_len; i++) {
        ret += gworkspace->sessions[i]->box.width + gworkspace->gap_x;
    }

    *out = ret;
    return 0;
}

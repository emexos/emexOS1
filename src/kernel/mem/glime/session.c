#include "glime.h"
#include <fonts/font_8x8.h>
#include <klib/debug/serial.h>

gsession_t *gsession_init(glime_t *glime, u8 *name, u64 width) {
    if (!glime) return NULL;

    gsession_t *session = (gsession_t *) glime_create(glime, sizeof(gsession_t));
    if (!session) return NULL;

    int seek = 0;
    while (name[seek] != '\0' && seek < SESSION_NAME_MAX - 1) {
        session->name[seek] = name[seek];
        seek++;
    }
    session->name[seek] = '\0';

    session->box.x      = 0;
    session->box.y      = 0;
    session->box.width  = width;
    session->box.height = glime->glres.height;

    session->kbrb = glime_keyboard_init(glime, 128);
    if (!session->kbrb) {
        glime_free(glime, (u64 *)session);
        return NULL;
    }

    return session;
}

int gsession_attach(glime_t *glime, gsession_t *gsession, u8 *workspace_name) {
    if (!glime || !gsession || !workspace_name) return 1;
    for (int i = 0; i < glime->workspaces_total; i++) {
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

        if (eq) {
            if (ws->sessions_len > ws->sessions_total) return 1;
            ws->sessions[ws->sessions_len] = gsession;
            ws->session_active = ws->sessions_len;
            ws->sessions_len++;
            gsession->gworkspace = ws;
            return 0;
        }
    }

    return 1;
}

int gsession_detach(gworkspace_t *gworkspace, gsession_t *gsession) {
    if (!gworkspace || !gsession) return 1;
    u8 *ssname = gsession->name;
    for (int i = 0; i < gworkspace->sessions_total; i++) {
        gsession_t *session = gworkspace->sessions[i];
        u8 *ssname_loop = session->name;
        int seek = 0;
        int eq = 1;
        while (ssname[seek] != '\0') {
            if (ssname_loop[seek] == '\0') {
                eq = 0;
                break;
            }
            if (ssname[seek] != ssname_loop[seek]) {
                eq = 0;
                break;
            }
            seek += 1;
        }

        if (eq) {
            gworkspace->sessions[i] = NULL;
            gworkspace->sessions_len--;
            gsession_clear(gsession, 0);
            return 0;
        }
    }

    return 1;
}


void gsession_clear(gsession_t *gsession, u32 color) {
    if (!gsession ||!gsession->gworkspace || !gsession->gworkspace->glime) return;

    u64 session_x = gsession->box.x;
    u64 session_y = gsession->box.y;
    u64 session_width = gsession->box.width;
    u64 session_height = gsession->box.height;

    u64 screen_height = gsession->gworkspace->glime->glres.height;
    u64 screen_width = gsession->gworkspace->glime->glres.width;

    u32 *fb = gsession->gworkspace->glime->framebuffer;

    for (u32 dy = 0; dy < session_height; dy++) {
        u64 screen_y = session_y + dy;
        for (u32 dx = 0; dx < session_width; dx++) {
            u64 screen_x = session_x + dx;

            if (screen_x < screen_width && screen_y < screen_height) {
                u64 screen_index = screen_y * screen_width + screen_x;
                fb[screen_index] = color;
            }
        }
    }
}


void gsession_put_at_string_dummy(gsession_t *gsession, u8 *string, u32 x, u32 y, u32 color) {
    if (!gsession ||!gsession->gworkspace || !gsession->gworkspace->glime) return;

    u64 session_x = gsession->box.x;
    u64 session_y = gsession->box.y;
    u64 session_width = gsession->box.width;
    u64 session_height = gsession->box.height;

    u64 screen_height = gsession->gworkspace->glime->glres.height;
    u64 screen_width = gsession->gworkspace->glime->glres.width;

    int seek = 0;
    int posx = x;
    int posy = y;

    u32 *fb = gsession->gworkspace->glime->framebuffer;

    while (string[seek] != '\0') {
        const u8 *glyph = font_8x8[(u8)string[seek]];

        for (int dy = 0; dy < 8; dy++) {
            u8 row = glyph[dy];
            for (int dx = 0; dx < 8; dx++) {
                if (row & (1 << (7 - dx))) {
                    // Calculate absolute screen coordinates
                    u64 screen_x = session_x + posx + dx;
                    u64 screen_y = session_y + posy + dy;

                    // Check bounds against session AND screen
                    if (screen_x < (session_x + session_width) &&
                        screen_y < (session_y + session_height) &&
                        screen_x < screen_width &&
                        screen_y < screen_height) {

                        u64 screen_index = screen_y * screen_width + screen_x;
                        fb[screen_index] = color;
                    }
                }
            }
        }

        if (string[seek] == '\n') {
            posx = x;
            posy += 10;
        } else {
            posx += 8;

            // Word wrap
            if (posx + 8 >= (int)session_width) {
                posx = x;
                posy += 10;
            }
        }

        if (posy + 8 >= (int)session_height) {
            break;
        }

        seek++;
    }
}

void gsession_put_at_char_dummy(gsession_t *gsession, u8 c, u32 x, u32 y, u32 color) {
    if (!gsession ||!gsession->gworkspace || !gsession->gworkspace->glime) return;

    u64 session_x = gsession->box.x;
    u64 session_y = gsession->box.y;
    u64 session_width = gsession->box.width;
    u64 session_height = gsession->box.height;

    u64 screen_height = gsession->gworkspace->glime->glres.height;
    u64 screen_width = gsession->gworkspace->glime->glres.width;

    u32 *fb = gsession->gworkspace->glime->framebuffer;

    const u8 *glyph = font_8x8[c];

    for (int dy = 0; dy < 8; dy++) {
        u8 row = glyph[dy];
        for (int dx = 0; dx < 8; dx++) {
            if (row & (1 << (7 - dx))) {
                // Calculate absolute screen coordinates
                u64 screen_x = session_x + x + dx;
                u64 screen_y = session_y + y + dy;

                // Check bounds against session AND screen
                if (screen_x < (session_x + session_width) &&
                    screen_y < (session_y + session_height) &&
                    screen_x < screen_width &&
                    screen_y < screen_height) {

                    u64 screen_index = screen_y * screen_width + screen_x;
                    fb[screen_index] = color;
                }
            }
        }
    }

}

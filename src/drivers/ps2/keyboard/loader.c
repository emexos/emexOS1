// loader.c
#include "loader.h"
//#include <kernel/include/reqs.h>
#include <memory/main.h>
#include <string/string.h>
#include <kernel/communication/serial.h>
#include <kernel/file_systems/vfs/vfs.h>
#include <config/files.h>

// Parse .map (emex keymap) file format
#define KEYMAP_DIR "/emr/config/keymaps/"

static int parse_keymap_data(const char *data, size_t size, keymap_t *km) {
    if (!data || !km || size == 0) return -1;

    // start to zero
    memset(km, 0, sizeof(keymap_t));

    const char *line = data;
    const char *end = data + size;

    while (line < end) {
        // skip space
        while (line < end && (*line == ' ' || *line == '\t' || *line == '\r')) line++;

        if (line >= end) break;

        if (*line == '#' || *line == '\n') {
            // skip comments and empty lines
            while (line < end && *line != '\n') line++;
            if (line < end) line++;
            continue;
        }

        // parse  in "0xSC = 'c' 'C'" format
        if (line + 2 < end && line[0] == '0' && line[1] == 'x') {
            u8 scancode = 0;
            line += 2;

            // (2 digits)
            for (int i = 0; i < 2 && line < end; i++) {
                scancode <<= 4;
                if      (*line >= '0' && *line <= '9') scancode |= (*line - '0');
                else if (*line >= 'A' && *line <= 'F') scancode |= (*line - 'A' + 10);
                else if (*line >= 'a' && *line <= 'f') scancode |= (*line - 'a' + 10);
                line++;
            }

            while (line < end && *line != '=') line++;
            if (line < end) line++; // skip '='
            while (line < end && (*line == ' ' || *line == '\t')) line++;

            // parse normal char
            if (line < end && *line == '\'') {
                line++;
                if (line < end) {
                    if (*line == '\\' && line + 1 < end) {
                        // Handle escape sequences
                        line++;
                        switch (*line) {
                            case 'n': km->normal[scancode] = '\n'; break;
                            case 't': km->normal[scancode] = '\t'; break;
                            case 'b': km->normal[scancode] = '\b'; break;
                            case 'r': km->normal[scancode] = '\r'; break;
                            case 'x': // \xHH hex escape
                                if (line + 2 < end) {
                                    line++;
                                    u8 hex = 0;
                                    for (int i = 0; i < 2 && line < end; i++) {
                                        hex <<= 4;
                                        if      (*line >= '0' && *line <= '9') hex |= (*line - '0');
                                        else if (*line >= 'A' && *line <= 'F') hex |= (*line - 'A' + 10);
                                        else if (*line >= 'a' && *line <= 'f') hex |= (*line - 'a' + 10);
                                        line++;
                                    }
                                    km->normal[scancode] = hex;
                                    line--; // increment
                                }
                                break;
                            default: km->normal[scancode] = *line; break;
                        }
                        line++;
                    } else {
                        km->normal[scancode] = *line++;
                    }

                    // skip closing quote
                    if (line < end && *line == '\'') line++;
                }
            }

            // skip whitespace
            while (line < end && (*line == ' ' || *line == '\t')) line++;

            //p arse shift character
            if (line < end && *line == '\'') {
                line++;
                if (line < end) {
                    if (*line == '\\' && line + 1 < end) {
                        line++;
                        switch (*line) {
                            case 'n': km->shift[scancode] = '\n'; break;
                            case 't': km->shift[scancode] = '\t'; break;
                            case 'b': km->shift[scancode] = '\b'; break;
                            case 'r': km->shift[scancode] = '\r'; break;
                            case 'x': // \xHH hex escape
                                if (line + 2 < end) {
                                    line++;
                                    u8 hex = 0;
                                    for (int i = 0; i < 2 && line < end; i++) {
                                        hex <<= 4;
                                        if      (*line >= '0' && *line <= '9') hex |= (*line - '0');
                                        else if (*line >= 'A' && *line <= 'F') hex |= (*line - 'A' + 10);
                                        else if (*line >= 'a' && *line <= 'f') hex |= (*line - 'a' + 10);
                                        line++;
                                    }
                                    km->shift[scancode] = hex;
                                    line--; // increment
                                }
                                break;
                            default: km->shift[scancode] = *line; break;
                        }
                        line++;
                    } else {
                        km->shift[scancode] = *line++;
                    }

                    if (line < end && *line == '\'') line++;
                }
            }
        }

        // skip to nect line
        while (line < end && *line != '\n') line++;
        if (line < end) line++;
    }

    return 0;
}

int keymap_load_from_module(const char *name, keymap_t *km) {
    if (!name || !km) return -1;

    // build path with us keymap
    char path[64];
    str_copy(path, KEYMAP_DIR);
    str_append(path, name);
    str_append(path, KEYMAP_FORMAT);

    int fd = fs_open(path, O_RDONLY);
    if (fd < 0) {
        log("[KEYMAP]", "not found: ", d);
        BOOTUP_PRINT(path, white());
        BOOTUP_PRINT("\n", white());
        return -1;
    }

    // read the whole file into a stack buffer
    // .map files are small (< 4KB)
    char buf[4096];
    ssize_t bytes = fs_read(fd, buf, sizeof(buf) - 1);
    fs_close(fd);

    if (bytes <= 0) return -1;
    buf[bytes] = '\0';

    return parse_keymap_data(buf, (size_t)bytes, km);
}

int keymap_modules_init(void) {
    return 0;
}

// loader.c
#include "loader.h"
#include <kernel/include/reqs.h>
#include <memory/main.h>
#include <string/string.h>
#include <kernel/communication/serial.h>
#include <kernel/file_systems/vfs/vfs.h>
#include <config/files.h>

// Parse .map (emex keymap) file format
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
            if (line < end && *line == '\n') line++;
            continue;
        }

        // parse  in "0xSC = 'c' 'C'" format
        if (line + 2 < end && line[0] == '0' && line[1] == 'x') {
            u8 scancode = 0;
            line += 2;

            // (2 digits)
            for (int i = 0; i < 2 && line < end; i++) {
                scancode <<= 4;
                if (*line >= '0' && *line <= '9') {
                    scancode |= (*line - '0');
                } else if (*line >= 'A' && *line <= 'F') {
                    scancode |= (*line - 'A' + 10);
                } else if (*line >= 'a' && *line <= 'f') {
                    scancode |= (*line - 'a' + 10);
                }
                line++;
            }

            while (line < end && *line != '=') line++; // =
            if (line < end && *line == '=') line++;
            while (line < end && (*line == ' ' || *line == '\t')) line++; // space

            // parse normal, between single quotes
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
                                        if (*line >= '0' && *line <= '9') {
                                            hex |= (*line - '0');
                                        } else if (*line >= 'A' && *line <= 'F') {
                                            hex |= (*line - 'A' + 10);
                                        } else if (*line >= 'a' && *line <= 'f') {
                                            hex |= (*line - 'a' + 10);
                                        }
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
                                        if (*line >= '0' && *line <= '9') {
                                            hex |= (*line - '0');
                                        } else if (*line >= 'A' && *line <= 'F') {
                                            hex |= (*line - 'A' + 10);
                                        } else if (*line >= 'a' && *line <= 'f') {
                                            hex |= (*line - 'a' + 10);
                                        }
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
        if (line < end && *line == '\n') line++;
    }

    return 0;
}

int keymap_load_from_module(const char *name, keymap_t *km) {
    if (!name || !km) return -1;

    if (!module_request.response || module_request.response->module_count == 0) {
        printf("[KEYMAP] No Limine modules available\n");
        return -1;
    }

    struct limine_module_response *response =
        (struct limine_module_response *)module_request.response;

    // Builds the expected filename
    char expected_name[64];
    str_copy(expected_name, name);
    str_append(expected_name, KEYMAP_FORMAT);

    // search for keymap module
    for (u64 i = 0; i < response->module_count; i++) {
        struct limine_file *module = response->modules[i];

        //Extract filename from path
        const char *filename = module->path;
        const char *last_slash = filename;
        for (const char *p = filename; *p; p++) {
            if (*p == '/') last_slash = p + 1;
        }
        filename = last_slash;

        // checks if this is our keymap
        if (str_equals(filename, expected_name)) {
            printf("[KEYMAP] Loading %s from module (%lu bytes)\n",
                   filename, module->size);

            return parse_keymap_data((const char*)module->address,
                                   module->size, km);
        }
    }

    printf("[KEYMAP] Module %s not found\n", expected_name);
    return -1;
}

int keymap_modules_init(void) {
    if (!module_request.response) {
        printf("[KEYMAP] No module response available\n");
        return -1;
    }

    struct limine_module_response *response =
        (struct limine_module_response *)module_request.response;

    printf("[KEYMAP] Found %lu Limine modules\n", response->module_count);

    // lists all keymap modules
    int keymap_count = 0;
    for (u64 i = 0; i < response->module_count; i++) {
        struct limine_file *module = response->modules[i];

        const char *filename = module->path;
        const char *last_slash = filename;
        for (const char *p = filename; *p; p++) {
            if (*p == '/') last_slash = p + 1;
        }
        filename = last_slash;

        // check if this is a keymap file
        if (str_contains(filename, KEYMAP_FORMAT)) {
            printf("[KEYMAP] Available: %s\n", filename);
            keymap_count++;
        }
    }

    printf("[KEYMAP] Total keymaps found: %d\n", keymap_count);
    return keymap_count;
}

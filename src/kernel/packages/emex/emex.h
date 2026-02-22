#ifndef EMEX_H
#define EMEX_H

#include <types.h>
#include <kernel/data/images/bmp.h>
#include <kernel/packages/elf/loader.h>
#include <kernel/user/ulime.h>

//
// EMX it's the emexOS app package format
// used for compatibility with [REDACTED]OS (raspberry pi OS)
// https://github.com/differrari/RedactedOS/blob/main/kernel/process/loading/elf_file.c
//
// current different things:
// package.info uses html
// app.elf is a fixed name
//

// those cant be changed otherwise compatibility will brake
#define EMEX_GEN_NAME  "app"
#define EMEX_ELF_NAME  EMEX_GEN_NAME "" ".elf"
#define EMEX_INFO_NAME "package.info"
#define EMEX_ICON_NAME "app_icon.bmp"
#define EMEX_RES_DIR   "resources"
#define EMEX_NEWEXT    ".emx"
#define EMEX_OK 0
#define EMEX_ERR_NOT_FOUND -1// directory doesn't exist
#define EMEX_ERR_NOT_EMX -2  // wrong extention
#define EMEX_ERR_NO_ELF -3   // app.elf is missing without it the .emx is useles....
#define EMEX_ERR_NO_INFO -4  // missing package.info
#define EMEX_ERR_BAD_INFO -5 // wrong package.info  (can happen when redacted OS apps are transfered)
#define EMEX_ERR_BAD_ELF -6  // failed to load elf
#define EMEX_ERR_NO_PROC -7  // failed to create process

// info parsed from package.info (html)
typedef struct {
    char title[64];// fixed length
    char author[64]; // fixed length
    char version[16]; // fixed length
    char description[256]; // fixed length
} emex_info_t;

// the full package struct that gets filled by emex_open()
typedef struct {
    char path[256]; // path to .emx
    emex_info_t info;
    bmp_image_t icon;// app icons are just bmp images
    int has_icon; // 1 if success
    int has_resources;
} emex_package_t;

// packreader
int emex_open_app(const char *path, emex_package_t *pkg);
void emex_close_app(emex_package_t *pkg);

// info parser
int emex_parse_info(const char *info_path, emex_info_t *info);

// image/icon parser
int emex_load_icon(const char *icon_path, bmp_image_t *icon);
void emex_draw_icon(bmp_image_t *icon, u32 x, u32 y);

// main file
int emex_launch_app(const char *path, ulime_proc_t **out_proc);
void emex_print_info(emex_package_t *pkg);

#endif

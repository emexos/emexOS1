#include "emex.h"

#include <kernel/data/html/html.h>
#include <kernel/file_systems/vfs/vfs.h>
#include <string/string.h>
#include <kernel/communication/serial.h>

//
// info parser/reader
//
// <html>
//   <head><title>Cool App</title></head>
//   <body>
//     <author>ein_franzose</author>
//     <version>3001.0</version>
//     <description>does cool stuff</description>
//   </body>
// </html>
//
// all fields except title are technically optional...
// but yeah better to have all those
//

// copies src into dst, but caps it at max_len-1 so we don't overflow
// nothing fancy, just safe copying
static void safe_copy(char *dst, const char *src, int max_len) {
    if (!dst || !src) return;
    int i = 0;
    while (src[i] != '\0' && i < max_len - 1) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

int emex_parse_info(const char *info_path, emex_info_t *info)
{
    if (!info_path || !info) return EMEX_ERR_BAD_INFO;

    // zero out the info struct first so we always have clean defaults
    str_copy(info->title,EMEX_ELF_NAME);
    str_copy(info->author,EMEX_GEN_NAME "" EMEX_NEWEXT);
    str_copy(info->version,"0");
    str_copy(info->description,"[empty]");

    int fd = fs_open(info_path, O_RDONLY);
    if (fd < 0) {
        printf("[EMX-INFO] can't open %s\n", info_path);
        return EMEX_ERR_NO_INFO;
    }

    // read it into a buffer
    char buf[2048]; // 2KB if we put all max lengths together it cant be more than this

    ssize_t bytes = fs_read(fd, buf, sizeof(buf) - 1);
    fs_close(fd);

    if (bytes <= 0) {
        printf("[EMX-INFO] %s is empty or unreadable\n", info_path);
        return EMEX_ERR_BAD_INFO;
    }
    buf[bytes] = '\0';

    // let the html parser do its thing
    // NOTE: htmlparse() modifies the buffer in place (replaces some chars with \0),
    // so we can't use "buf" for anything else after this call
    htmlparse(buf);
    // we need to create a new buffer now

    // grab whatever fields we can find and copy them into info
    // if a field is missing we just keep the default from above

    const char *title = htmlget("title");
    const char *author = htmlget("author");
    const char *version = htmlget("version");
    const char *desc = htmlget("description");
    if (title && *title) {
        safe_copy(info->title, title, sizeof(info->title));
    } else {
        printf("[EMX-INFO] warning: no <title> found in package.info\n");
    }
    if (author && *author) {
        safe_copy(info->author, author, sizeof(info->author));
    } else {
        printf("[EMX-INFO] warning: no <author> tag in package.info\n");
    }
    if (version && *version) {
        safe_copy(info->version, version, sizeof(info->version));
    } else {
        printf("[EMX-INFO] warning: no <version> tag in package.info\n");
    }
    if (desc && *desc) {
        safe_copy(info->description, desc, sizeof(info->description));
    } else {
        printf("[EMX-INFO] warning: no <description> tag in package.info\n");
    }


    return EMEX_OK;
}

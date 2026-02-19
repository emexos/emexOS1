#include "initrd.h"
#include <kernel/packages/cpio/cpio.h>
#include <kernel/file_systems/vfs/vfs.h>
#include <kernel/include/reqs.h>       // module_request
#include <string/string.h>
#include <kernel/communication/serial.h>
#include <kernel/graph/theme.h>
#include <theme/doccr.h>

int initrd_load(void)
{
    log("[INITRD]", "looking for " INITRD_MODULE_NAME "...\n", d);

    if (!module_request.response ||
        module_request.response->module_count == 0)
    {
        log("[INITRD]", "no Limine modules available\n", warning);
        return -1;
    }

    struct limine_module_response *resp = module_request.response;
    struct limine_file *initrd_file = NULL;

    for (u64 i = 0; i < resp->module_count; i++) {
        const char *path = resp->modules[i]->path;

        // extract just the filename from the Limine path
        const char *fname = path;
        for (const char *p = path; *p; p++) {
            if (*p == '/') fname = p + 1;
        }

        if (str_equals(fname, INITRD_MODULE_NAME)) {
            initrd_file = resp->modules[i];
            break;
        }
    }

    if (!initrd_file) {
        log("[INITRD]", INITRD_MODULE_NAME " not found in Limine modules\n", warning);
        //hcf();
        return -1;
    }



    {
        char buf[48];
        str_copy(buf, "found at 0x");
        str_from_hex(buf + str_len(buf), (u64)initrd_file->address);
        str_append(buf, " (");
        str_append_uint(buf, (u32)initrd_file->size);
        str_append(buf, " bytes)\n");
        log("[INITRD]", buf, d);
    }


    // CPIO newc magic
    const u8 *raw = (const u8 *)initrd_file->address;
    if (initrd_file->size < 6 ||
        raw[0]!='0' || raw[1]!='7' || raw[2]!='0' ||
        raw[3]!='7' || raw[4]!='0' || (raw[5]!='1' && raw[5]!='2'))
    {
        log("[INITRD]", "the initrd file uses the wrong format, need to be newc\n", error);
        return -1;
    }


    int extracted = cpio_extract_to_vfs(
        raw,
        (u64)initrd_file->size,
        "/"
    );

    if (extracted < 0) {
        log("[INITRD]", "extraction failed\n", error);
        //hcf();
        return -1;
    }

    //log("[INITRD]", "mounted at the root\n", success);
    return extracted;
}

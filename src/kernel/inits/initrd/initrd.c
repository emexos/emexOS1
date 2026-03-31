#include "initrd.h"
#include <kernel/packages/cpio/cpio.h>
#include <kernel/file_systems/vfs/vfs.h>
#include <kernel/inits/fs/init.h>
#include <kernel/include/reqs.h>       // module_request
#include <string/string.h>
#include <kernel/communication/serial.h>
#include <kernel/graph/theme.h>
#include <theme/doccr.h>

static int is_cpio_newc(const u8 *raw, u64 size) {
	if (size < 6) return 0;

	return (
		raw[0]=='0' &&
		raw[1]=='7' &&
		raw[2]=='0' &&
		raw[3]=='7' &&
		raw[4]=='0' &&
		(
			raw[5]=='1' || raw[5]=='2'
		)
	);
}

static struct limine_file *find_module(const char *name)
{
    struct limine_module_response *resp = module_request.response;

    for (u64 i = 0; i < resp->module_count; i++) {
        const char *path = resp->modules[i]->path;

        const char *fname = path;
        for (const char *p = path; *p; p++) {
            if (*p == '/') fname = p + 1;
        }

        if (str_equals(fname, name)) {
            return resp->modules[i];
        }
    }

    return NULL;
}

static void log_module(const char *tag, struct limine_file *file)
{
    char buf[64];

    str_copy(buf, "found at 0x");
    str_from_hex(buf + str_len(buf), (u64)file->address);
    str_append(buf, " (");
    str_append_uint(buf, (u32)file->size);
    str_append(buf, " bytes)\n");

    log(tag, buf, d);
}

int initrd_load(void)
{
    log("[INITRD]", "loading initrds...\n", d);

    if (!module_request.response ||
        module_request.response->module_count == 0)
    {
        log("[INITRD]", "no Limine modules available\n", warning);
        return -1;
    }

    struct limine_file *initrd  = find_module("initrd.cpio");
    struct limine_file *initrdh = find_module("initrdh.cpio");

    if (!initrd) {
        log("[INITRD]", "initrd.cpio not found\n", error);
        return -1;
    }

    log_module("[INITRD] initrd", initrd);

    // put initrd.cpio to /
    const u8 *raw = (const u8 *)initrd->address;

    if (!is_cpio_newc(raw, initrd->size)) {
        log("[INITRD]", "initrd has wrong format (need newc)\n", error);
        return -1;
    }

    int res = cpio_extract_to_vfs(
        raw,
        (u64)initrd->size,
        "/"
    );

    if (res < 0) {
        log("[INITRD]", "initrd extraction failed\n", error);
        return -1;
    }

    log("[INITRD]", "root filesystem loaded\n", success);

    // put initrdh.cpio to /home
    if (initrdh) {
        log_module("[INITRD] initrdh", initrdh);

        const u8 *rawh = (const u8 *)initrdh->address;

        if (!is_cpio_newc(rawh, initrdh->size)) {
            log("[INITRD]", "initrdh has wrong format (need newc)\n", error);
            return -1;
        }

        //fs_mkdir(HOME);

        int resh = cpio_extract_to_vfs(
            rawh,
            (u64)initrdh->size,
            "/"
        );

        if (resh < 0) {
            log("[INITRD]", "initrdh extraction failed\n", error);
            return -1;
        }

        log("[INITRD]", "home filesystem loaded\n", success);
    } else {
        log("[INITRD]", "initrdh.cpio not found (no /home)\n", warning);
    }

    return 0;
}
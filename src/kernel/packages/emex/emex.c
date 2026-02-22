#include "emex.h"

#include <kernel/file_systems/vfs/vfs.h>
#include <kernel/packages/elf/loader.h>
#include <kernel/user/ulime.h>
#include <kernel/proc/proc_manager.h>
#include <config/user.h>
#include <string/string.h>
#include <kernel/communication/serial.h>

#if ENABLE_ULIME
extern proc_manager_t *proc_mgr;
extern ulime_t *ulime;
#endif

// prints everything about a package to serial
void emex_print_info(emex_package_t *pkg)
{
    if (!pkg) return;
    printf("[EMX] path:        %s\n", pkg->path);
    printf("[EMX] title:       %s\n", pkg->info.title);
    printf("[EMX] author:      %s\n", pkg->info.author);
    printf("[EMX] version:     %s\n", pkg->info.version);
    printf("[EMX] description: %s\n", pkg->info.description);
    printf("[EMX] has icon:    %s\n", pkg->has_icon ?  "yes" : "no");
    printf("[EMX] resources:   %s\n", pkg->has_resources ? "yes" : "no");
}

// opens the package, reads the elf, launches with process
int emex_launch_app(const char *path, ulime_proc_t **out_proc)
{
    if (!path) return EMEX_ERR_NOT_FOUND;

    if (out_proc) *out_proc = NULL;

    printf("[EMX] launching: %s\n", path);

    emex_package_t pkg;
    int open_result = emex_open_app(path, &pkg);
    if (open_result != EMEX_OK) {
        printf("[EMX] failed to open package (code %d)\n", open_result);
        return open_result;
    }

    emex_print_info(&pkg);

    char elf_path[256];
    str_copy(elf_path, path);
    int plen = str_len(elf_path);
    if (plen > 0 && elf_path[plen - 1] != '/') str_append(elf_path, "/");
    str_append(elf_path, EMEX_ELF_NAME);

    int fd = fs_open(elf_path, O_RDONLY);
    if (fd < 0) {
        printf("[EMX] can't open %s\n", elf_path);
        emex_close_app(&pkg);
        return EMEX_ERR_NO_ELF;
    }

    // 512kb max elf size, static to avoid depending on klime here
    #define EMX_ELF_MAX_SIZE (512 * 1024)
    static u8 elf_buf[EMX_ELF_MAX_SIZE];
    ssize_t elf_size = fs_read(fd, elf_buf, EMX_ELF_MAX_SIZE);
    fs_close(fd);

    if (elf_size <= 0) {
        printf("[EMX] elf file is empty or unreadable\n");
        emex_close_app(&pkg);
        return EMEX_ERR_BAD_ELF;
    }

    printf("[EMX] read %d bytes of elf\n", (int)elf_size);

#if ENABLE_ULIME
    if (!ulime || !proc_mgr) {
        printf("[EMX] ulime/proc_mgr not ready\n");
        emex_close_app(&pkg);
        return EMEX_ERR_NO_PROC;
    }

    u8 proc_name[32];
    int name_len = str_len(pkg.info.title);
    if (name_len > 31) name_len = 31;
    int i;
    for (i = 0; i < name_len; i++) {
        proc_name[i] = (u8)pkg.info.title[i];
    }
    proc_name[i] = '\0';

    ulime_proc_t *proc = proc_create_proc(proc_mgr, proc_name, 0, USERPRIORITY);
    if (!proc) {
        printf("[EMX] failed to create process for '%s'\n", pkg.info.title);
        emex_close_app(&pkg);
        return EMEX_ERR_NO_PROC;
    }

    printf("[EMX] created process pid=%llu for '%s'\n", proc->pid, pkg.info.title);

    int elf_result = elf_load(proc, elf_buf, (u64)elf_size);
    if (elf_result != 0) {
        printf("[EMX] elf_load failed for '%s'\n", pkg.info.title);
        emex_close_app(&pkg);
        return EMEX_ERR_BAD_ELF;
    }

    printf("[EMX] '%s' ready at entry=0x%llX (state=PROC_READY)\n",
           pkg.info.title, proc->entry_point);

    if (out_proc) *out_proc = proc;

#else
    printf("[EMX] ENABLE_ULIME is off, can't run the app\n");
#endif

    emex_close_app(&pkg);
    return EMEX_OK;
}

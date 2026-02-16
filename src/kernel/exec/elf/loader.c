// elf loader from a month ago (i wonder this works.. i mean i think it works)
#include "loader.h"

#include <string/string.h>
#include <memory/main.h>
#include <kernel/communication/serial.h>

int elf_load(ulime_proc_t *proc, u8 *elf_data, u64 size) {
    if (!proc || !elf_data || size < sizeof(elf_header_t)) {
        printf("[ELF] Invalid parameters\n");
        return -1;
    }

    elf_header_t *ehdr = (elf_header_t*)elf_data;

    if (ehdr->ident.magic != ELF_MAGIC) {
        printf("[ELF] Invalid magic\n");
        return -1;
    }

    if (ehdr->ident.class != 2) {
        printf("[ELF] not 64-bit ELF\n");
        return -1;
    }

    if (ehdr->ident.endian != 1) { // 1 = little-endian
        printf("[ELF] unsupported endianness\n");
        return -1;
    }

    if (ehdr->machine != 0x3E) {
        printf("[ELF] not x86-64\n");
        return -1;
    }

    printf("[ELF] Loading '%s'\n", proc->name);
    printf("  Entry: 0x%lX\n", ehdr->entry);

    if (ehdr->phoff + ehdr->phnum * sizeof(elf_program_header_t) > size) {
        printf("[ELF] Program headers exceed ELF size\n");
        return -1;
    }

    elf_program_header_t *phdr = (elf_program_header_t*)(elf_data + ehdr->phoff);

    for (u16 i = 0; i < ehdr->phnum; i++) {
        if (phdr[i].type != PT_LOAD) continue;

        u64 vaddr = phdr[i].vaddr;
        u64 offset = phdr[i].offset;
        u64 filesz = phdr[i].filesz;
        u64 memsz = phdr[i].memsz;

        if (vaddr < proc->heap_base || vaddr + memsz > proc->heap_base + proc->heap_size) {
            printf("[ELF] Segment outside heap\n");
            return -1;
        }

        if (offset + filesz > size) {
            printf("[ELF] Segment exceeds ELF size\n");
            return -1;
        }

        if (filesz > 0) {
            memcpy((void*)vaddr, elf_data + offset, filesz);
            printf("  Copied %lu bytes to 0x%lX\n", filesz, vaddr);
        }

        if (memsz > filesz) {
            memset((void*)(vaddr + filesz), 0, memsz - filesz);
            printf("  Zeroed %lu bytes at 0x%lX\n", memsz - filesz, vaddr + filesz);
        }

        u8 *code_check = (u8*)vaddr;
        printf("  First 16 bytes: ");
        for (int j = 0; j < 16 && j < (int)filesz; j++) {
            printf("%02X ", code_check[j]);
        }
        printf("\n");
    }

    proc->entry_point = ehdr->entry;
    proc->state = PROC_READY;

    printf("[ELF] success\n");
    return 0;
}

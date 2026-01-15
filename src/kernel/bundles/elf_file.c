#include "elf_file.h"
#include <memory/main.h>
#include <kernel/mem/klime/klime.h>
#include <kernel/mem/phys/physmem.h>
#include <kernel/mem/paging/paging.h>
#include <kernel/communication/serial.h>

// Validate ELF file magic and basic structure
int elf_validate(const u8 *data, u64 size) {
    if (!data || size < sizeof(elf64_ehdr_t)) {
        return 0;
    }

    const elf64_ehdr_t *ehdr = (const elf64_ehdr_t *)data;

    // Check ELF magic number
    if (ehdr->e_ident[0] != 0x7F ||
        ehdr->e_ident[1] != 'E' ||
        ehdr->e_ident[2] != 'L' ||
        ehdr->e_ident[3] != 'F') {
        printf("[ELF] Invalid magic number\n");
        return 0;
    }

    // Check for 64-bit ELF
    if (ehdr->e_ident[4] != ELF_CLASS_64) {
        printf("[ELF] Not a 64-bit ELF file\n");
        return 0;
    }

    // Check for little-endian
    if (ehdr->e_ident[5] != ELF_DATA_LSB) {
        printf("[ELF] Not a little-endian ELF file\n");
        return 0;
    }

    // Check version
    if (ehdr->e_ident[6] != 1) {
        printf("[ELF] Invalid ELF version\n");
        return 0;
    }

    // Check machine type (x86-64)
    if (ehdr->e_machine != EM_X86_64) {
        printf("[ELF] Not an x86-64 executable\n");
        return 0;
    }

    return 1;
}

// Parse ELF header
int elf_parse_header(const u8 *data, elf64_ehdr_t *ehdr) {
    if (!data || !ehdr) {
        return 0;
    }

    memcpy(ehdr, data, sizeof(elf64_ehdr_t));
    return 1;
}

// Get entry point address
int elf_get_entry_point(const u8 *data, u64 *entry) {
    if (!data || !entry) {
        return 0;
    }

    const elf64_ehdr_t *ehdr = (const elf64_ehdr_t *)data;
    *entry = ehdr->e_entry;
    return 1;
}

// Load ELF file into memory
elf_load_result_t elf_load_file(const u8 *data, u64 size, void *klime) {
    elf_load_result_t result = {0};

    if (!elf_validate(data, size)) {
        printf("[ELF] Validation failed\n");
        return result;
    }

    const elf64_ehdr_t *ehdr = (const elf64_ehdr_t *)data;

    printf("[ELF] Loading executable:\n");
    printf("  Type: %s\n", elf_get_type_string(ehdr->e_type));
    printf("  Machine: %s\n", elf_get_machine_string(ehdr->e_machine));
    printf("  Entry: 0x%llx\n", ehdr->e_entry);
    printf("  Program Headers: %d\n", ehdr->e_phnum);

    // Find load boundaries
    u64 min_addr = (u64)-1;
    u64 max_addr = 0;

    for (int i = 0; i < ehdr->e_phnum; i++) {
        const elf64_phdr_t *phdr = (const elf64_phdr_t *)(data + ehdr->e_phoff + i * ehdr->e_phentsize);

        if (phdr->p_type != PT_LOAD) {
            continue;
        }

        if (phdr->p_vaddr < min_addr) {
            min_addr = phdr->p_vaddr;
        }

        u64 seg_end = phdr->p_vaddr + phdr->p_memsz;
        if (seg_end > max_addr) {
            max_addr = seg_end;
        }
    }

    if (min_addr == (u64)-1 || max_addr == 0) {
        printf("[ELF] No loadable segments found\n");
        return result;
    }

    u64 total_size = max_addr - min_addr;
    printf("[ELF] Load region: 0x%llx - 0x%llx (size: %llu bytes)\n",
           min_addr, max_addr, total_size);

    // Allocate memory for the program
    u64 *load_mem = klime_alloc((klime_t *)klime, 1, total_size);
    if (!load_mem) {
        printf("[ELF] Failed to allocate memory\n");
        return result;
    }

    memset(load_mem, 0, total_size);
    u64 load_base = (u64)load_mem;

    // Load each PT_LOAD segment
    for (int i = 0; i < ehdr->e_phnum; i++) {
        const elf64_phdr_t *phdr = (const elf64_phdr_t *)(data + ehdr->e_phoff + i * ehdr->e_phentsize);

        if (phdr->p_type != PT_LOAD) {
            continue;
        }

        printf("[ELF] Loading segment %d:\n", i);
        printf("  Virtual: 0x%llx\n", phdr->p_vaddr);
        printf("  File size: %llu\n", phdr->p_filesz);
        printf("  Mem size: %llu\n", phdr->p_memsz);
        printf("  Flags: %c%c%c\n",
               (phdr->p_flags & PF_R) ? 'R' : '-',
               (phdr->p_flags & PF_W) ? 'W' : '-',
               (phdr->p_flags & PF_X) ? 'X' : '-');

        // Calculate destination address
        u64 dest = load_base + (phdr->p_vaddr - min_addr);

        // Copy file data
        if (phdr->p_filesz > 0) {
            memcpy((void *)dest, data + phdr->p_offset, phdr->p_filesz);
        }

        // Zero out remaining memory (BSS)
        if (phdr->p_memsz > phdr->p_filesz) {
            memset((void *)(dest + phdr->p_filesz), 0, phdr->p_memsz - phdr->p_filesz);
        }
    }

    // Calculate adjusted entry point
    result.entry_point = load_base + (ehdr->e_entry - min_addr);
    result.load_base = load_base;
    result.load_size = total_size;
    result.success = 1;

    printf("[ELF] Load complete\n");
    printf("  Base: 0x%llx\n", result.load_base);
    printf("  Entry: 0x%llx\n", result.entry_point);
    printf("  Size: %llu bytes\n", result.load_size);

    return result;
}

// Get human-readable type string
const char* elf_get_type_string(u16 type) {
    switch (type) {
        case ET_NONE: return "None";
        case ET_REL:  return "Relocatable";
        case ET_EXEC: return "Executable";
        case ET_DYN:  return "Shared Object";
        case ET_CORE: return "Core File";
        default:      return "Unknown";
    }
}

// Get human-readable machine string
const char* elf_get_machine_string(u16 machine) {
    switch (machine) {
        case EM_NONE:   return "None";
        case EM_386:    return "Intel 80386";
        case EM_X86_64: return "AMD x86-64";
        default:        return "Unknown";
    }
}

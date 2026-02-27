// elf loader from a month ago (i wonder this works.. i mean i think it works)
// added relocation
#include "loader.h"

#include <string/string.h>
#include <memory/main.h>
#include <kernel/communication/serial.h>
#include <kernel/mem/paging/paging.h>

int elf_load(ulime_proc_t *proc, u8 *elf_data, u64 size) {
    if (!proc || !elf_data || size < sizeof(elf_header_t)) {
        printf("[ELF] Invalid parameters\n");
        return -1;
    }

    elf_header_t *ehdr = (elf_header_t*)elf_data;

    // check ELF magic: 0x7F 'E' 'L' 'F'
    if (ehdr->ident.magic[0] != 0x7F ||
        ehdr->ident.magic[1] != 'E' ||
        ehdr->ident.magic[2] != 'L' ||
        ehdr->ident.magic[3] != 'F') {
        printf("[ELF] Invalid magic: %02X %02X %02X %02X\n",
               ehdr->ident.magic[0], ehdr->ident.magic[1],
               ehdr->ident.magic[2], ehdr->ident.magic[3]);
        return -1;
    }

    if (ehdr->ident.class != 2) {
        printf("[ELF] not 64-bit ELF\n");
        return -1;
    }

    if (ehdr->ident.endian != 1) { // 1 == little-endian
        printf("[ELF] unsupported endianness\n");
        return -1;
    }

    if (ehdr->machine != 0x3E) {
        printf("[ELF] not x86-64\n");
        return -1;
    }

    if (ehdr->phoff + ehdr->phnum * sizeof(elf_program_header_t) > size) {
        printf("[ELF] Program headers exceed ELF size\n");
        return -1;
    }

    elf_program_header_t *phdr = (elf_program_header_t*)(elf_data + ehdr->phoff);

    u64 min_vaddr = (u64)-1;
    u64 max_vaddr = 0;
    for (u16 i = 0; i < ehdr->phnum; i++) {
        if (phdr[i].type == PT_LOAD) {
            if (phdr[i].vaddr < min_vaddr) min_vaddr = phdr[i].vaddr;
            u64 end = phdr[i].vaddr + phdr[i].memsz;
            if (end > max_vaddr) max_vaddr = end;
        }
    }
    if (min_vaddr == (u64)-1) {
        printf("[ELF] No loadable segments found\n");
        return -1;
    }

    u64 load_offset = proc->heap_base - min_vaddr;
    u64 hhdm_offset = proc->ulime->hpr->offset;

    printf("[ELF] Loading '%s'\n", proc->name);
    printf("  ELF base addr: 0x%lX\n", min_vaddr);
    printf("  Process heap:  0x%lX\n", proc->heap_base);
    printf("  Load offset:   0x%lX\n", load_offset);
    printf("  Original entry: 0x%lX\n", ehdr->entry);

    // load PT_LOAD segments
    // writes via HHDM so kernel CR3 doesnt matter
    for (u16 i = 0; i < ehdr->phnum; i++) {
        if (phdr[i].type != PT_LOAD) continue;

        u64 vaddr = phdr[i].vaddr + load_offset;  // relocated virtual address
        u64 offset = phdr[i].offset;
        u64 filesz = phdr[i].filesz;
        u64 memsz  = phdr[i].memsz;

        printf("  Segment %d:\n", i);
        printf("    Original vaddr: 0x%lX\n", phdr[i].vaddr);
        printf("    Relocated to:   0x%lX\n", vaddr);
        printf("    File size:      %lu bytes\n", filesz);
        printf("    Mem size:       %lu bytes\n", memsz);

        // bounds check against heap
        if (vaddr < proc->heap_base || vaddr + memsz > proc->heap_base + proc->heap_size) {
            printf("[ELF] ERROR: Segment outside heap bounds\n");
            printf("    Segment range: 0x%lX - 0x%lX\n", vaddr, vaddr + memsz);
            printf("    Heap range:    0x%lX - 0x%lX\n",
                   proc->heap_base, proc->heap_base + proc->heap_size);
            return -1;
        }
        if (offset + filesz > size) {
            printf("[ELF] Segment exceeds ELF size\n");
            return -1;
        }

        // convert virtual address to kernel-accessible HHDM address
        u64 phys_dest  = proc->phys_heap + (vaddr - proc->heap_base);
        void *hhdm_dest = (void *)(phys_dest + hhdm_offset);

        if (filesz > 0) {
            memcpy(hhdm_dest, elf_data + offset, filesz);
            printf("    Copied %lu bytes\n", filesz);
        }
        if (memsz > filesz) {
            memset((u8 *)hhdm_dest + filesz, 0, memsz - filesz);
            printf("    Zeroed %lu bytes\n", memsz - filesz);
        }

        // print first bytes for debug (read via HHDM too)
        u8 *code = (u8 *)hhdm_dest;
        printf("    First bytes: ");
        for (int j = 0; j < 16 && j < (int)filesz; j++) {
            printf("%02X ", code[j]);
        }
        printf("\n");
    }

    // relocate entry point
    proc->entry_point = ehdr->entry + load_offset;
    proc->state = PROC_READY;

    // if the ELF was relocated it also maps the original VMA into the process PML4
    // so that absolute addresses baked into the binary still work at runtime
    if (load_offset != 0) {
        u64 total_size = (max_vaddr - min_vaddr + 0xFFF) & ~0xFFFULL;
        u64 pages = total_size / 0x1000;
        u64 heap_flags = PTE_PRESENT | PTE_WRITABLE | PTE_USER;

        printf("[ELF] remapping original VMA 0x%lX (%lu pages) to same phys\n",
               min_vaddr, pages);

        for (u64 p = 0; p < pages; p++) {
            u64 phys      = proc->phys_heap + (p * 0x1000);
            u64 orig_virt = min_vaddr + (p * 0x1000);
            // maps into the processes own PML4 NOT the global kernel one
            paging_map_page_proc(proc->ulime->hpr, proc->pml4_phys,
                                 orig_virt, phys, heap_flags);
        }

        printf("[ELF] original VMA alias mapped\n");
    }

    printf("[ELF] Relocated entry point: 0x%lX\n", proc->entry_point);
    printf("[ELF] Load successful\n");
    return 0;
}

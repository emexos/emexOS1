#pragma once
#include <types.h>
#include <kernel/user/ulime.h>


#define PT_LOAD 1
#define ELF_MAGIC 0x7F454C46


typedef struct {
    u32 magic;
    u8  class;
    u8  endian;
    u8  version;
    u8  osabi;
    u64 reserved;
} __attribute__((packed)) elf_ident_t;

typedef struct {
    elf_ident_t ident;
    u16 type;
    u16 machine;
    u32 version;
    u64 entry;
    u64 phoff;
    u64 shoff;
    u32 flags;
    u16 ehsize;
    u16 phentsize;
    u16 phnum;
    u16 shentsize;
    u16 shnum;
    u16 shstrndx;
} __attribute__((packed)) elf_header_t;

typedef struct {
    u32 type;
    u32 flags;
    u64 offset;
    u64 vaddr;
    u64 paddr;
    u64 filesz;
    u64 memsz;
    u64 align;
} __attribute__((packed)) elf_program_header_t;

int elf_load(ulime_proc_t *proc, u8 *elf_data, u64 size);

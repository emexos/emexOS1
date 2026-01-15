#ifndef ELF_FILE_H
#define ELF_FILE_H

#include <types.h>

// ELF64 Magic Number
#define ELF_MAGIC 0x464C457F  // 0x7F, 'E', 'L', 'F'

// ELF Class
#define ELF_CLASS_32 1
#define ELF_CLASS_64 2

// ELF Data Encoding
#define ELF_DATA_LSB 1  // Little Endian
#define ELF_DATA_MSB 2  // Big Endian

// ELF Types
#define ET_NONE   0  // No file type
#define ET_REL    1  // Relocatable
#define ET_EXEC   2  // Executable
#define ET_DYN    3  // Shared object
#define ET_CORE   4  // Core file

// Machine Types
#define EM_NONE   0   // No machine
#define EM_386    3   // Intel 80386
#define EM_X86_64 62  // AMD x86-64

// Program Header Types
#define PT_NULL    0  // Unused entry
#define PT_LOAD    1  // Loadable segment
#define PT_DYNAMIC 2  // Dynamic linking info
#define PT_INTERP  3  // Interpreter path
#define PT_NOTE    4  // Auxiliary info
#define PT_SHLIB   5  // Reserved
#define PT_PHDR    6  // Program header table
#define PT_TLS     7  // Thread-Local Storage

// Program Header Flags
#define PF_X 0x1  // Execute
#define PF_W 0x2  // Write
#define PF_R 0x4  // Read

// Section Header Types
#define SHT_NULL     0   // Unused
#define SHT_PROGBITS 1   // Program data
#define SHT_SYMTAB   2   // Symbol table
#define SHT_STRTAB   3   // String table
#define SHT_RELA     4   // Relocation entries with addends
#define SHT_HASH     5   // Symbol hash table
#define SHT_DYNAMIC  6   // Dynamic linking info
#define SHT_NOTE     7   // Notes
#define SHT_NOBITS   8   // Program space with no data (bss)
#define SHT_REL      9   // Relocation entries without addends
#define SHT_SHLIB    10  // Reserved
#define SHT_DYNSYM   11  // Dynamic linker symbol table

// Section Header Flags
#define SHF_WRITE     0x1  // Writable
#define SHF_ALLOC     0x2  // Occupies memory during execution
#define SHF_EXECINSTR 0x4  // Executable

// ELF64 Header
typedef struct {
    u8  e_ident[16];     // Magic number and other info
    u16 e_type;          // Object file type
    u16 e_machine;       // Architecture
    u32 e_version;       // Object file version
    u64 e_entry;         // Entry point virtual address
    u64 e_phoff;         // Program header table file offset
    u64 e_shoff;         // Section header table file offset
    u32 e_flags;         // Processor-specific flags
    u16 e_ehsize;        // ELF header size in bytes
    u16 e_phentsize;     // Program header table entry size
    u16 e_phnum;         // Program header table entry count
    u16 e_shentsize;     // Section header table entry size
    u16 e_shnum;         // Section header table entry count
    u16 e_shstrndx;      // Section header string table index
} __attribute__((packed)) elf64_ehdr_t;

// ELF64 Program Header
typedef struct {
    u32 p_type;    // Segment type
    u32 p_flags;   // Segment flags
    u64 p_offset;  // Segment file offset
    u64 p_vaddr;   // Segment virtual address
    u64 p_paddr;   // Segment physical address
    u64 p_filesz;  // Segment size in file
    u64 p_memsz;   // Segment size in memory
    u64 p_align;   // Segment alignment
} __attribute__((packed)) elf64_phdr_t;

// ELF64 Section Header
typedef struct {
    u32 sh_name;       // Section name (string tbl index)
    u32 sh_type;       // Section type
    u64 sh_flags;      // Section flags
    u64 sh_addr;       // Section virtual addr at execution
    u64 sh_offset;     // Section file offset
    u64 sh_size;       // Section size in bytes
    u32 sh_link;       // Link to another section
    u32 sh_info;       // Additional section information
    u64 sh_addralign;  // Section alignment
    u64 sh_entsize;    // Entry size if section holds table
} __attribute__((packed)) elf64_shdr_t;

// ELF64 Symbol Table Entry
typedef struct {
    u32 st_name;   // Symbol name (string tbl index)
    u8  st_info;   // Symbol type and binding
    u8  st_other;  // Symbol visibility
    u16 st_shndx;  // Section index
    u64 st_value;  // Symbol value
    u64 st_size;   // Symbol size
} __attribute__((packed)) elf64_sym_t;

// ELF Load Result
typedef struct {
    u64 entry_point;    // Program entry point
    u64 load_base;      // Base address where loaded
    u64 load_size;      // Total size loaded
    int success;        // 1 if successful, 0 otherwise
} elf_load_result_t;

// Function declarations
int elf_validate(const u8 *data, u64 size);
int elf_parse_header(const u8 *data, elf64_ehdr_t *ehdr);
int elf_get_entry_point(const u8 *data, u64 *entry);
elf_load_result_t elf_load_file(const u8 *data, u64 size, void *klime);
const char* elf_get_type_string(u16 type);
const char* elf_get_machine_string(u16 machine);

#endif

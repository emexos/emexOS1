OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)

/* force one single PT_LOAD segment (RWX) so the emex loader doesnt miss .rodata/.data which would otherwise end up
*  in separate segments the loader never processes
*/
PHDRS { load PT_LOAD FLAGS(7); }

SECTIONS
{
    . = 0x40000000;
    .text   : { *(.text._start) *(.text*) } :load
    .rodata : { *(.rodata*) } :load
    .data   : { *(.data*) } :load
    .bss    : { *(.bss*) *(COMMON) } :load
    /DISCARD/ : { *(.eh_frame) *(.note*) *(.comment) }
}

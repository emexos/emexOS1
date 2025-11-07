[BITS 64]

global gdt_flush
global tss_flush

; void gdt_flush(u64 gdt_ptr)
gdt_flush:
    lgdt [rdi]          ; Load GDT

    ; Reload segment registers
    mov ax, 0x10        ; Kernel Data Segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Far return to reload CS
    pop rdi             ; Save return address
    mov rax, 0x08       ; Kernel Code Segment
    push rax
    push rdi
    o64 retfq           ; Far return in 64-bit mode

; void tss_flush(u16 selector)
tss_flush:
    ltr di              ; Load Task Register
    ret

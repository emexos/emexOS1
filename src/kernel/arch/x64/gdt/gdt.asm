[BITS 64]

global gdt_flush
global tss_flush

; void gdt_flush(u64 gdt_ptr)
gdt_flush:
    lgdt [rdi]          ; load GDT
    mov ax, 0x10        ; kernel Data Segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    pop rdi             ; save return address
    mov rax, 0x08       ; Kernel Code Segment
    push rax
    push rdi
    o64 retfq           ; far return in 64-bit mode

; void tss_flush(u16 selector)
tss_flush:
    ltr di              ;load Task Register
    ret

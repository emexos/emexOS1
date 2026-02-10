[BITS 64]

global idt_flush

; void idt_flush(u64 idt_ptr)
idt_flush:
    lidt [rdi]      ; IDT from RDI
    ret

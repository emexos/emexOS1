.section .text
.global _start


_start:
    xor  %rbp, %rbp
    xor  %edi, %edi
    xor  %esi, %esi
    call main
    mov  %rax, %rdi
    call exit
    hlt

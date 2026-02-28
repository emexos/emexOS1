.section .text
.global _start


_start:
    xor  %rbp, %rbp    // clear frame pointer (ABI requirement)
    mov  (%rsp),  %rdi // argc == *rsp
    lea  8(%rsp), %rsi // argv == rsp+8
    call main
    mov  %rax, %rdi
    call exit
    hlt

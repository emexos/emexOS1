bits 64

global syscall_entry
global user_rsp
global user_rcx
global user_r11
global user_cr3
global user_rbx
global user_rbp
global user_r12
global user_r13
global user_r14
global user_r15

extern syscall_handler

syscall_entry:
    mov [rel user_rsp], rsp
    mov [rel user_rcx], rcx
    mov [rel user_r11], r11

    push rax
    mov rax, cr3
    mov [rel user_cr3], rax
    pop rax

    mov [rel user_rbx], rbx
    mov [rel user_rbp], rbp
    mov [rel user_r12], r12
    mov [rel user_r13], r13
    mov [rel user_r14], r14
    mov [rel user_r15], r15

    lea rsp, [rel kernel_stack_top]

    push r11
    push rcx
    push rbx
    push rbp
    push r12
    push r13
    push r14
    push r15

    mov rcx, rdx ; arg3
    mov rdx, rsi ; arg2
    mov rsi, rdi ; arg1
    mov rdi, rax ; syscall_num

    call syscall_handler

    ; result is in rax

    pop r15
    pop r14
    pop r13
    pop r12
    pop rbp
    pop rbx
    pop rcx ; user rip
    pop r11 ; rflags

    mov rsp, [rel user_rsp]
    o64 sysret     ; RIP = rcx, RFLAGS = r11, CPL → 3

section .data
align 16
user_rsp: dq 0
user_rcx: dq 0
user_r11: dq 0
user_cr3: dq 0
user_rbx: dq 0
user_rbp: dq 0
user_r12: dq 0
user_r13: dq 0
user_r14: dq 0
user_r15: dq 0

section .bss
align 16
kernel_stack_bottom:
    resb 16384
kernel_stack_top:

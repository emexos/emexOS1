bits 64

global syscall_entry
extern syscall_handler



syscall_entry:
    ; save user stack pointer
    mov [rel user_rsp], rsp
    lea rsp, [rel kernel_stack_top] ; not mov

    ; save registers on kernel stack
    push r11 ; rflags
    push rcx ; user rip
    push rbx
    push rbp
    push r12
    push r13
    push r14
    push r15

    mov rcx, rdx ; rcx = arg3 (count)
    mov rdx, rsi ; rdx = arg2 (buf)
    mov rsi, rdi ; rsi = arg1 (fd)
    mov rdi, rax ; rdi = syscall_num

    call syscall_handler ; from C

    ; result is in rax

    ; restore registers
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbp
    pop rbx
    pop rcx ; user rip
    pop r11 ; rflags

    ; restore user stack
    mov rsp, [rel user_rsp]

    ; return to userspace with SYSRET
    ; rcx = return address, r11 = rflags
    o64 sysret

section .data
align 16
user_rsp: dq 0


section .bss
align 16
kernel_stack_bottom:
    resb 16384          ; 16KB kernel stack
kernel_stack_top:

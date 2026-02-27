bits 64

global resume_parent_sysret
resume_parent_sysret:
    cli

    ;read stack args before we lose rsp
    mov rax, [rsp + 8]  ;r12
    mov [rel .r12], rax
    mov rax, [rsp + 16] ;r13
    mov [rel .r13], rax
    mov rax, [rsp + 24] ;r14
    mov [rel .r14], rax
    mov rax, [rsp + 32] ;r15
    mov [rel .r15], rax

    ;switch to parents page table
    ;kernel upperhalf stays valid after this switch
    mov cr3, rcx

    ;set up sysret registers:
    ; rcx = return RIP (was in rdi)
    ; r11 = RFLAGS (was in rdx)
    ; rsp = user stack (was in rsi)
    mov rcx, rdi ; RIP >> rcx (sysret reads RIP from here)
    mov r11, rdx ; RFLAGS >> r11 (sysret reads RFLAGS from here)
    mov rsp, rsi ; restore user stack


    ;restore callee-saved registers
    mov rbx, r8
    mov rbp, r9
    mov r12, [rel .r12]
    mov r13, [rel .r13]
    mov r14, [rel .r14]
    mov r15, [rel .r15]

    ;rax = 0 >> execve() return value the parent sees (success)
    xor rax, rax
    xor rdi, rdi
    xor rsi, rsi
    xor rdx, rdx
    ; rcx/r11/rsp already set above, don't zero them
    xor r8,  r8
    xor r9,  r9
    xor r10, r10

    o64 sysret ; RIP = rcx, RFLAGS = r11, CPL >> 3

section .data
align 8
.r12: dq 0
.r13: dq 0
.r14: dq 0
.r15: dq 0

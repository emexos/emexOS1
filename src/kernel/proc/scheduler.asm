[BITS 64]

global scheduler_context_switch

scheduler_context_switch:
    ; RDI = old_state
    ; RSI = new_state

    test rdi, rdi
    jz .load_new_state

    mov [rdi + 0], r15
    mov [rdi + 8], r14
    mov [rdi + 16], r13
    mov [rdi + 24], r12
    mov [rdi + 32], r11
    mov [rdi + 40], r10
    mov [rdi + 48], r9
    mov [rdi + 56], r8
    mov [rdi + 64], rbp
    mov [rdi + 72], rdi
    mov [rdi + 80], rsi
    mov [rdi + 88], rdx
    mov [rdi + 96], rcx
    mov [rdi + 104], rbx
    mov [rdi + 112], rax

    mov rax, [rsp]
    mov [rdi + 136], rax

    lea rax, [rsp + 8]
    mov [rdi + 160], rax

    pushfq
    pop rax
    mov [rdi + 152], rax

.load_new_state:
    test rsi, rsi
    jz .done

    mov r15, [rsi + 0]
    mov r14, [rsi + 8]
    mov r13, [rsi + 16]
    mov r12, [rsi + 24]
    mov r11, [rsi + 32]
    mov r10, [rsi + 40]
    mov r9, [rsi + 48]
    mov r8, [rsi + 56]
    mov rbp, [rsi + 64]
    mov rdx, [rsi + 88]
    mov rcx, [rsi + 96]
    mov rbx, [rsi + 104]
    mov rax, [rsi + 112]

    mov r10, [rsi + 152]
    push r10
    popfq

    mov rsp, [rsi + 160]

    mov rdi, [rsi + 72]
    mov r10, [rsi + 80]

    push qword [rsi + 136]
    mov rsi, r10

    ret

.done:
    ret

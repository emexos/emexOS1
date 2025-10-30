; stubs
[BITS 64]

extern isr_handler

; Macro > ISRs without Error Codes
%macro ISR_NOERRCODE 1
global isr%1
isr%1:
    push 0              ; Dummy Error Code
    push %1             ; Interrupt Number
    jmp isr_common_stub
%endmacro

; Macro > ISRs for Error Codes
%macro ISR_ERRCODE 1
global isr%1
isr%1:
    push %1             ; Interrupt Number
    jmp isr_common_stub
%endmacro

; ISR Stubs (0-31)
ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_ERRCODE   17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_ERRCODE   30
ISR_NOERRCODE 31

; Common ISR Handler
isr_common_stub:
    ; save regs
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    ; stack pointer to 'handler'
    mov rdi, rsp
    call isr_handler

    ; Regs back
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    ; remove ErrorCode, INT Number from the stack
    add rsp, 16

    iretq

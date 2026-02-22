.section .rodata
msg:
    .ascii "[emr_system] starting... main usersystem\n"
msg_len = . - msg

.section .text
.global _start

_start:
    # write(1, msg, msg_len)
    mov $1, %rax
    mov $1, %rdi
    lea msg(%rip), %rsi
    mov $msg_len, %rdx
    syscall

    # execve("/login.emx") or whatever
.loop:
    pause
    jmp .loop

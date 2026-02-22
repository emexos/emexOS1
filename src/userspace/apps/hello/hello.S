.section .rodata
msg:
    .ascii "hello from userspace!\n"
msg_len = . - msg

.section .text
.global _start

_start:
    # write(1, msg, msg_len)
    mov $1, %rax # WRITE
    mov $1, %rdi # stdout
    lea msg(%rip), %rsi
    mov $msg_len, %rdx
    syscall

    # exit(0)
    mov $60, %rax # EXIT
    xor %rdi, %rdi # 0
    syscall

    # should never reach here
    hlt

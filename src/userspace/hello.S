.section .text
.global _start

_start:
    # write(1, msg, 26)
    mov $1, %rax  # SYS_WRITE
    mov $1, %rdi  # fd = STDOUT
    lea msg(%rip), %rsi # buf = msg
    mov $26, %rdx # count = 26
    syscall

    # exit(0)
    mov $60, %rax  # SYS_EXIT
    xor %rdi, %rdi # status = 0
    syscall

    hlt

.section .rodata
msg:
    .ascii "hello world from usermode\n"

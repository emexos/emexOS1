.section .text
.global _start

_start:
    # just exit immediately with code 42 to test if user mode works
    mov $60, %rax # EXIT
    mov $42, %rdi # status == 42
    syscall

    # should never reach here
    hlt

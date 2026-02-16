.section .text
.global _start

_start:
    # Just exit immediately with code 42 to test if user mode works
    mov $60, %rax      # SYS_EXIT
    mov $42, %rdi      # status = 42
    syscall

    # Should never reach here
    hlt

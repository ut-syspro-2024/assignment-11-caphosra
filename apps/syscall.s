.intel_syntax noprefix
.global syscall
syscall:
    int 0x80
    ret

.global syscall_puts
syscall_puts:
    mov rsi, rdi
    mov rdi, 0x1
    call syscall
    ret

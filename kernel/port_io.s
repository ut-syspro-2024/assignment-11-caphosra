.intel_syntax noprefix

.global read_pm_timer
read_pm_timer:
    mov dx, di
    in eax, dx
    ret

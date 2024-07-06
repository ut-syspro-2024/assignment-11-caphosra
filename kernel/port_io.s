.intel_syntax noprefix

.global read_pm_timer
read_pm_timer:
    mov dx, di
    in eax, dx
    ret

.global read_pci_config_data
read_pci_config_data:
    mov eax, edi
    mov dx, 0xcf8
    out dx, eax
    mov dx, 0xcfc
    in eax, dx
    ret

.global io_read_b
io_read_b:
    mov dx, di
    in al, dx
    ret

.global io_read_w
io_read_w:
    mov dx, di
    in ax, dx
    ret

.global io_read_d
io_read_d:
    mov dx, di
    in eax, dx
    ret

.global io_write_w
io_write_w:
    mov dx, di
    mov ax, si
    out dx, ax
    ret

.global io_write_d
io_write_d:
    mov dx, di
    mov eax, esi
    out dx, eax
    ret

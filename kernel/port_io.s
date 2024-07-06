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

.global read_from_port
read_from_port:
    mov dx, di
    in al, dx
    ret

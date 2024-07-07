.intel_syntax noprefix

.global init_app_stack
init_app_stack:
    // Save kernel cr3 and switch to task.
    mov rbx, cr3
    mov cr3, rdx
    mov rcx, rsp
    mov rsp, rdi
    // Calculate feature rsp and store to rdx.
    mov rdx, rsp
    sub rdx, 48
    // Add the context to the stack.
    mov rax, ss
    push rax
    push rdx
    pushfq
    mov rax, cs
    push rax
    push rsi
    // Return to the main stack.
    mov rsp, rcx
    mov cr3, rbx
    ret

.intel_syntax noprefix

.global lapic_intr_handler
lapic_intr_handler:
  push rax
  push rbx
  push rcx
  push rdx
  push rdi
  push rsi
  push rbp
  mov rdi, rsp
  call lapic_intr_handler_internal
  pop rbp
  pop rsi
  pop rdi
  pop rdx
  pop rcx
  pop rbx
  pop rax
  iretq

.global syscall_handler
syscall_handler:
  // Save Registers
  push rbx
  push rcx
  push rdx
  push rdi
  push rsi
  push rbp
  call syscall_handler_internal
  // Resume
  pop rbp
  pop rsi
  pop rdi
  pop rdx
  pop rcx
  pop rbx
  iretq

.global page_fault_handler
page_fault_handler:
  // Take an error code
  push rax
  push rbx
  push rcx
  push rdx
  push rdi
  push rsi
  push rbp
  mov rdi, cr2
  call page_fault_handler_internal
  pop rbp
  pop rsi
  pop rdi
  pop rdx
  pop rcx
  pop rbx
  pop rax
  add rsp, 8
  iretq

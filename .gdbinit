target remote :1234
file kernel/kernel.elf
set substitute-path /work/kernel ./kernel
b init_tasks
b init_task
b schedule
b syscall_handler_internal
b page_fault_handler_internal

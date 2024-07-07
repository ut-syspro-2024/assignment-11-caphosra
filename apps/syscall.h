typedef enum SYSCALL {
    SYSCALL_PUTS = 1,
} SYSCALL;

unsigned long long syscall_puts(char* str);

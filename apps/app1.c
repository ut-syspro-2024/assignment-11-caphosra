#include "syscall.h"

unsigned long long text_section_top = 0x104000000;

void app1() {
  int* p = (int*)((0x3ull << 39) + 0x123ull);
  *p = 42;

  while (1) {
    char *str = "Hello from app1\r";
    syscall_puts(str);

    volatile int i = 100000000;
    while (i--);
  }

  asm volatile ("jmp *%0" :: "m"(text_section_top));
}

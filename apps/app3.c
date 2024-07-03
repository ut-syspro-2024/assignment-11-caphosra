#include "syscall.h"

unsigned long long text_section_top = 0x106000000;

void app3() {
  while (1) {
    char *str = "Hello from app3\r";
    syscall_puts(str);

    volatile int i = 100000000;
    while (i--);
  }

  asm volatile ("jmp *%0" :: "m"(text_section_top));
}

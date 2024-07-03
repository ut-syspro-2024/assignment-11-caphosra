#include "syscall.h"

unsigned long long text_section_top = 0x105000000;

void app2() {
  while (1) {
    char *str = "Hello from app2\r";
    syscall_puts(str);

    volatile int i = 100000000;
    while (i--);
  }

  asm volatile ("jmp *%0" :: "m"(text_section_top));
}

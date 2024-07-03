#pragma once

#define INTR_PAGE_FAULT 0x0e
#define INTR_LAPIC 0x20
#define INTR_SYSCALL 0x80

#define IDT_ATTR_INTR_GATE (0x0e << 8)
#define IDT_ATTR_TRAP_GATE (0x0f << 8)
#define IDT_ATTR_ENABLED (1 << 15)

void init_intr();

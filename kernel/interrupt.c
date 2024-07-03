#include "interrupt.h"
#include "lapic_timer.h"

struct InterruptDescriptor {
  unsigned short offset_lo;
  unsigned short segment;
  unsigned short attribute;
  unsigned short offset_mid;
  unsigned int offset_hi;
  unsigned int reserved;
} __attribute__((packed));

struct InterruptDescriptor IDT[256];

void lapic_intr_handler();

void syscall_handler();

void page_fault_handler();

static void load_idt_to_idtr() {
  // Set idtr
  unsigned short table_limit = sizeof(struct InterruptDescriptor) * 256 - 1;
  unsigned long long base_addr = (unsigned long long)IDT;
  unsigned char data[10];
  *(unsigned short*)data = table_limit;
  *(unsigned long long*)(data + 2) = base_addr;
  asm volatile ("lidt %0" :: "m"(data));
}

static void register_intr_handler(unsigned char index, unsigned long long offset, unsigned short segment, unsigned short attribute) {
  IDT[index].offset_hi = offset >> 32 & 0xFFFFFFFF;
  IDT[index].offset_mid = offset >> 16 & 0xFFFF;
  IDT[index].offset_lo = offset & 0xFFFF;

  IDT[index].reserved = 0;
  IDT[index].segment = segment;
  IDT[index].attribute = attribute;
}

void init_intr() {
  // Get segment register value
  unsigned short cs;
  asm volatile ("mov %%cs, %0" : "=r"(cs));

  void* lapic_intr_handler_addr;
  asm volatile ("lea lapic_intr_handler(%%rip), %0" : "=r"(lapic_intr_handler_addr));

  void* syscall_handler_addr;
  asm volatile ("lea syscall_handler(%%rip), %[handler]" : [handler]"=r"(syscall_handler_addr));

  void* page_fault_handler_addr;
  asm volatile ("lea page_fault_handler(%%rip), %[handler]" : [handler]"=r"(page_fault_handler_addr));


  // Register Local APIC handler
  register_intr_handler(INTR_LAPIC, (unsigned long long)lapic_intr_handler_addr, cs, IDT_ATTR_INTR_GATE | IDT_ATTR_ENABLED);

  // Register syscall handler
  register_intr_handler(INTR_SYSCALL, (unsigned long long)syscall_handler_addr, cs, IDT_ATTR_TRAP_GATE | IDT_ATTR_ENABLED);

  // Register page fault handler
  register_intr_handler(INTR_PAGE_FAULT, (unsigned long long)page_fault_handler_addr, cs, IDT_ATTR_TRAP_GATE | IDT_ATTR_ENABLED);

  // Tell CPU the location of IDT
  load_idt_to_idtr();

  // Set IF bit in RFLAGS register
  asm volatile ("sti");
}

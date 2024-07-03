#include "util.h"
#include "lapic_timer.h"
#include "sched.h"
#include "memory.h"

#pragma GCC diagnostic ignored "-Wunused-parameter"

#define STACK_SIZE 0x1000

#define APP_ENTRY 0x040000000
#define STACK_BOT 0x041000000

struct Task {
  unsigned long long sp;
  unsigned long long cr3;
};
struct Task tasks[TASK_NUM];
unsigned int current_task;

extern void init_app_stack(unsigned char* stack_bottom, unsigned long long rip, unsigned long long task_cr3);

static void init_task(int idx, unsigned char *stack_bottom, unsigned long long rip) {
  init_app_stack(stack_bottom, rip, task_cr3s[idx]);

  tasks[idx].sp = ((unsigned long long)stack_bottom) - 0x30 - 0x30;
  tasks[idx].cr3 = task_cr3s[idx];
}

void init_tasks() {
  init_task(1, (unsigned char*)STACK_BOT, (unsigned long long)APP_ENTRY);
  init_task(2, (unsigned char*)STACK_BOT, (unsigned long long)APP_ENTRY);

  current_task = 0;
  tasks[0].cr3 = task_cr3s[0];
  unsigned long long cr3_data = tasks[0].cr3;
  unsigned long long sp = (unsigned long long)STACK_BOT;
  unsigned long long rip = (unsigned long long)APP_ENTRY;
  asm volatile (
    "mov %0, %%cr3\n"
    "mov %1, %%rsp\n"
    "jmp *%2" :: "a"(cr3_data), "m"(sp), "m"(rip)
  );
}

void schedule(unsigned long long sp) {
  puts("schedule() called\r");

  tasks[current_task].sp = sp;
  current_task++;
  current_task %= TASK_NUM;
  unsigned long long new_sp = tasks[current_task].sp;
  unsigned long long cr3_data = tasks[current_task].cr3;

  puts_n("switching to ");
  puth(current_task, 1);

  lapic_set_eoi();
  asm volatile (
    "mov %0, %%rax\n"
    "mov %1, %%rsp\n"
    "mov %%rax, %%cr3\n"
    "pop %%rbp\n"
    "pop %%rsi\n"
    "pop %%rdi\n"
    "pop %%rdx\n"
    "pop %%rcx\n"
    "pop %%rbx\n"
    "pop %%rax\n"
    "iretq":: "m"(cr3_data), "m"(new_sp)
  );
}

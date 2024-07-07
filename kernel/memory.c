#include "memory.h"
#include "sched.h"
#include "util.h"

struct Entry {
  unsigned int present : 1;
  unsigned int writable : 1;
  unsigned int user_accessible : 1;
  unsigned int write_through_caching : 1;
  unsigned int disable_cache : 1;
  unsigned int accessed : 1;
  unsigned int dirty : 1;
  unsigned int huge_page : 1;
  unsigned int global : 1;
  unsigned int available1 : 3;
  unsigned long long physical_address : 40;
  unsigned int available2 : 11;
  unsigned int no_execute : 1;
} __attribute__((packed));

unsigned long long task_cr3s[TASK_NUM];
unsigned long long kernel_cr3;

struct Entry PML4s[TASK_NUM][512] __attribute__((aligned(4096)));
struct Entry PDPs[TASK_NUM][512] __attribute__((aligned(4096)));
struct Entry PDs[TASK_NUM][512] __attribute__((aligned(4096)));
struct Entry PTs[TASK_NUM][8][512] __attribute__((aligned(4096)));

struct Entry kernel_PD[512] __attribute__((aligned(4096)));
struct Entry kernel_PTs[8][512] __attribute__((aligned(4096)));

struct Entry io_PD[512] __attribute__((aligned(4096)));
struct Entry fb_PT[512] __attribute__((aligned(4096)));
struct Entry lapic_PT[512] __attribute__((aligned(4096)));

#define GET_PHYS_PTR(addr) (((unsigned long long)(addr)) >> 12)

void init_virtual_memory() {
  // Save kernel cr3 register value
  asm volatile ("mov %%cr3, %0" : "=r"(kernel_cr3));

  // Initialize virtual memories for kernel.
  for (int idx1 = 0; idx1 < 8; idx1++) {
    for (int idx2 = 0; idx2 < 512; idx2++) {
      // Initialize PTs.
      kernel_PTs[idx1][idx2].present = 1;
      kernel_PTs[idx1][idx2].writable = 1;
      kernel_PTs[idx1][idx2].physical_address = GET_PHYS_PTR(0x100000000) + idx1 * 512 + idx2;
    }

    // Initialize PD.
    kernel_PD[idx1].present = 1;
    kernel_PD[idx1].writable = 1;
    kernel_PD[idx1].physical_address = GET_PHYS_PTR(&kernel_PTs[idx1]);
  }

  // Initialize fb.
  for (int idx = 0; idx < 512; idx++) {
    fb_PT[idx].present = 1;
    fb_PT[idx].writable = 1;
    fb_PT[idx].physical_address = GET_PHYS_PTR(0x0c0000000) + idx;
  }

  io_PD[0].present = 1;
  io_PD[0].writable = 1;
  io_PD[0].physical_address = GET_PHYS_PTR(fb_PT);

  // Initialize lapic.
  lapic_PT[0].present = 1;
  lapic_PT[0].writable = 1;
  lapic_PT[0].physical_address = 0x0fee00000 >> 12;

  io_PD[503].present = 1;
  io_PD[503].writable = 1;
  io_PD[503].physical_address = GET_PHYS_PTR(lapic_PT);

  // Initialize virtual memories for applications.
  for (int task = 0; task < TASK_NUM; task++) {
    unsigned long long app_addr_root = 0x104000000 + task * 0x1000000;

    for (int idx1 = 0; idx1 < 8; idx1++) {
      // Initialize PTs.
      for (int idx2 = 0; idx2 < 512; idx2++) {
        PTs[task][idx1][idx2].present = 1;
        PTs[task][idx1][idx2].writable = 1;
        PTs[task][idx1][idx2].physical_address = GET_PHYS_PTR(app_addr_root) + idx1 * 521 + idx2;
      }

      // Initialize PDs.
      PDs[task][idx1].present = 1;
      PDs[task][idx1].writable = 1;
      PDs[task][idx1].physical_address = GET_PHYS_PTR(&PTs[task]);
    }

    // Initialize PDPs.
    PDPs[task][1].present = 1;
    PDPs[task][1].writable = 1;
    PDPs[task][1].physical_address = GET_PHYS_PTR(&PDs[task]);

    PDPs[task][3].present = 1;
    PDPs[task][3].writable = 1;
    PDPs[task][3].physical_address = GET_PHYS_PTR(io_PD);

    PDPs[task][4].present = 1;
    PDPs[task][4].writable = 1;
    PDPs[task][4].physical_address = GET_PHYS_PTR(kernel_PD);

    // Initialize PML4.
    PML4s[task][0].present = 1;
    PML4s[task][0].writable = 1;
    PML4s[task][0].physical_address = GET_PHYS_PTR(&PDPs[task]);

    // Set cr3
    task_cr3s[task] = GET_PHYS_PTR(&PML4s[task]) << 12;
  }
}

#define DEMAND_PAGING_PAGE 0x107000000

struct Entry demanded_PDP[512] __attribute__((aligned(4096)));
struct Entry demanded_PD[512] __attribute__((aligned(4096)));
struct Entry demanded_PT[512] __attribute__((aligned(4096)));

void page_fault_handler_internal(unsigned long long cr2) {
  struct Entry cr2_entry;
  *((unsigned long long*)&cr2_entry) = cr2;

  puts_n("page fault detected at app");
  puth(current_task + 1, 1);

  unsigned long long required_addr = cr2_entry.physical_address;
  puts_n("required addr: ");
  puth(required_addr, 28);

  int PML4_index = required_addr >> 27;
  int PDP_index = (required_addr >> 18) % 512;
  int PD_index = (required_addr >> 9) % 512;
  int PT_index = required_addr % 512;

  PML4s[current_task][PML4_index].present = 1;
  PML4s[current_task][PML4_index].writable = 1;
  PML4s[current_task][PML4_index].physical_address = GET_PHYS_PTR(demanded_PDP);

  demanded_PDP[PDP_index].present = 1;
  demanded_PDP[PDP_index].writable = 1;
  demanded_PDP[PDP_index].physical_address = GET_PHYS_PTR(demanded_PD);

  demanded_PD[PD_index].present = 1;
  demanded_PD[PD_index].writable = 1;
  demanded_PD[PD_index].physical_address = GET_PHYS_PTR(demanded_PT);

  demanded_PT[PT_index].present = 1;
  demanded_PT[PT_index].writable = 1;
  demanded_PT[PT_index].physical_address = GET_PHYS_PTR(DEMAND_PAGING_PAGE);

  puts("allocated a new page");
}

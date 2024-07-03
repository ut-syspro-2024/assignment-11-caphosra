#include "lapic_timer.h"
#include "pm_timer.h"
#include "util.h"
#include "interrupt.h"

#define COUNT_MAX 0xffffffff

volatile unsigned int *lvt_timer = (unsigned int *)0xfee00320;
volatile unsigned int *initial_count = (unsigned int *)0xfee00380;
volatile unsigned int *current_count = (unsigned int *)0xfee00390;
volatile unsigned int *divide_config = (unsigned int *)0xfee003e0;

unsigned int lapic_timer_freq_khz = 0;

volatile unsigned int *lapic_eoi = (unsigned int *)0xfee000b0;

#define LVT_TIMER_MASK (1 << 16)
#define LVT_TIMER_MODE_PERIODIC (0b01 << 17)

#define DIV_CONF_1_2 0b0000
#define DIV_CONF_1_4 0b0001
#define DIV_CONF_1_1 0b1011

void (*reserved_callback)(unsigned long long);

unsigned int measure_lapic_freq_khz() {
  // Initialize LAPIC One-Shot timer
  *lvt_timer = LVT_TIMER_MASK;
  *divide_config = DIV_CONF_1_1;
  *initial_count = INT32_MAX;

  // Wait 1000ms
  pm_timer_wait_millisec(1000);

  // Calculate LAPIC Freq.

  // The size of initial count and current count of LAPIC timer is 32bit.
  // Reference: Intel SDM Vol. 3A 11-17
  unsigned int timer = *current_count;
  lapic_timer_freq_khz = (INT32_MAX - timer) / 1000;

  return lapic_timer_freq_khz;
}

void lapic_periodic_exec(unsigned int msec, void *callback) {
  if (!lapic_timer_freq_khz) {
    puts("Call measure_lapic_freq_khz() beforehand.");
    return;
  }

  reserved_callback = callback;

  // Set LAPIC Periodic Timer
  *lvt_timer = INTR_LAPIC | LVT_TIMER_MODE_PERIODIC;
  *divide_config = DIV_CONF_1_1;
  *initial_count = msec * lapic_timer_freq_khz;
}

void lapic_intr_handler_internal(unsigned long long sp) {
  // Set End of Interrupt
  reserved_callback(sp);
}

void lapic_set_eoi() {
  *lapic_eoi = 1;
}

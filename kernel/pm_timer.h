#pragma once

#include "hardware.h"

extern const unsigned int freq_fz;
extern unsigned short pm_timer_blk;
extern char pm_timer_is_32;

#define PM_TIMER_MAX_32 (unsigned int)((1LLU << 32) * 1000 / freq_fz);
#define PM_TIMER_MAX_24 (unsigned int)((1LLU << 24) * 1000 / freq_fz);

void init_acpi_pm_timer(struct RSDP* rsdp);
void pm_timer_wait_millisec(unsigned int msec);
unsigned int read_pm_timer(unsigned short pm_timer_blk);

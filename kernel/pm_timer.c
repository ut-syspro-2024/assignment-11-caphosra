#include "pm_timer.h"
#include "hardware.h"
#include "util.h"

#define FADT_SIGNATURE "FACP"

const unsigned int freq_fz = 3579545;
static struct RSDP* global_rsdp;
unsigned short pm_timer_blk = 0;
char pm_timer_is_32 = 0;

void init_acpi_pm_timer(struct RSDP* rsdp) {
    global_rsdp = rsdp;
    struct XSDT* xsdt = (struct XSDT*)rsdp->xsdt_address;
    unsigned int xsdt_tables_length = (xsdt->sdth.length - 36) / 8;
    for (unsigned int idx = 0; idx < xsdt_tables_length; idx++) {
        struct SDTH* sdth = *(struct SDTH**)(((unsigned long long)xsdt) + 36 + idx * 8);
        if (!strcmp_len(sdth->signature, FADT_SIGNATURE, 4)) {
            struct FADT* fadt = (struct FADT*)sdth;
            pm_timer_blk = fadt->PM_TMR_BLK;
            pm_timer_is_32 = fadt->flags >> 0x8 & 0x1;
            return;
        }
    }
}

void pm_timer_wait_millisec(unsigned int msec) {
    if (pm_timer_is_32) { msec %= PM_TIMER_MAX_32; }
    else msec %= PM_TIMER_MAX_24;

    unsigned int time_waiting = msec * freq_fz / 1000;

    unsigned int start = read_pm_timer(pm_timer_blk);
    unsigned long long end = (long long)start + time_waiting;
    while (1) {
        unsigned int now = (unsigned long long)read_pm_timer(pm_timer_blk);
        if (start > now) now += pm_timer_is_32 ? (1LLU << 32) : (1 << 24);
        if (now > end) {
            return;
        }
    }
}

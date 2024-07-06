#include "util.h"

#define VIRTIO_NET_ID 0x10001AF4

extern int read_pci_config_data(int reg);

unsigned int search_virtio_net() {
    for (int bus = 0; bus < 0x100; bus++) {
        for (int device = 0; device < 32; device++) {
            int config_addr = (1 << 31) + (bus << 16) + (device << 11);
            int data = read_pci_config_data(config_addr);
            if (data == VIRTIO_NET_ID) {
                return config_addr;
            }
        }
    }
    return 0;
}

void print_mac_addr(short io_addr) {
    for (int idx = 0; idx < 6; idx++) {
        unsigned char num = io_read_b(io_addr + idx);
        puth_n(num, 2);
        if (idx != 5) {
            puts_n(":");
        }
    }
}

void init_virtio_driver() {
    unsigned int config_addr = search_virtio_net();
    if (!config_addr) {
        puts("failed to find virtio.");
        return;
    }
    puts("virtio found");

    puts_n("config_addr: ");
    puth(config_addr, 8);

    int base_addr = read_pci_config_data(config_addr | 0x10) ^ 0x1;

    puts_n("mac addr: ");
    print_mac_addr(base_addr + 20);
    puts_n("\n");
}

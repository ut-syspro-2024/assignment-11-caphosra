#include "util.h"

typedef unsigned short le16;
typedef unsigned int le32;
typedef unsigned long long le64;

#define VIRTQ_SIZE 0x10

struct virtq_desc {
    le64 addr;
    le32 len;
    le16 flags;
    le16 next;
};

struct virtq_avail {
#define VIRTQ_AVAIL_F_NO_INTERRUPT 1
    le16 flags;
    le16 idx;
    le16 ring[VIRTQ_SIZE];
    // le16 ring[VIRTQ_SIZE];
    // le16 used_event;
};

unsigned int get_virtq_avail_offset(int queue_size) {
    return sizeof(struct virtq_desc) * queue_size;
}

unsigned int get_virtq_avail_size(int queue_size) {
    return sizeof(le16) * (queue_size + 3);
}

struct virtq_used_elem {
    le32 id;
    le32 len;
};

struct virtq_used {
    le16 flags;
    le16 idx;
    struct virtq_used_elem ring[];
    // struct virtq_used_elem ring[VIRTQ_SIZE];
    // le16 avail_event;
};

unsigned int get_virtq_used_size(int queue_size) {
    return sizeof(le16) * 3 + sizeof(struct virtq_used_elem) * queue_size;
}

unsigned int get_padded_size(int size) {
    if (size % 0x1000 == 0) return size;
    return ((size >> 12) + 1) << 12;
}

unsigned int get_used_offset(int queue_size) {
    unsigned int desc_size = get_virtq_avail_offset(queue_size);
    unsigned int avail_size = get_virtq_avail_size(queue_size);
    unsigned int padded = get_padded_size(desc_size + avail_size);
    return padded;
}

unsigned int get_virtq_size(int queue_size) {
    return get_used_offset(queue_size) + get_virtq_used_size(queue_size);
}

unsigned char* rx_queue = (unsigned char*)0xbeef000;
unsigned char* tx_queue = (unsigned char*)0xcafe000;

unsigned char* rx_buffer = (unsigned char*)0xdead000;
#define RX_BUFFER_SIZE 0x1000

struct virtio_net_hdr {
#define VIRTIO_NET_HDR_F_NEEDS_CSUM 1
    unsigned char flags;
    unsigned char gso_type;
    le16 hdr_len;
    le16 gso_size;
    le16 csum_start;
    le16 csum_offset;
    // le16 num_buffers;
};

#define VIRTIO_NET_ID 0x10001AF4

#define VIRTIO_DEVICE_FEATURES_OFFSET 0x04
#define VIRTIO_QUEUE_ADDR_OFFSET 0x08
#define VIRTIO_NOTIFY_QUEUE_OFFSET 0x10
#define VIRTIO_QUEUE_SIZE_OFFSET 0x0c
#define VIRTIO_QUEUE_SELECT_OFFSET 0x0e
#define VIRTIO_DEVICE_STATUS_OFFSET 0x12

#define VIRTIO_MAC_OFFSET 0x14
#define VIRTIO_QUEUE_RX 0
#define VIRTIO_QUEUE_TX 1
#define VIRTIO_NET_F_CSUM (1 << 0)
#define VIRTIO_NET_F_MAC (1 << 5)

#define VIRTIO_STATUS_ACKNOWLEDGE 1
#define VIRTIO_STATUS_DRIVER 2
#define VIRTIO_STATUS_DRIVER_OK 4
#define VIRTIO_STATUS_FEATURES_OK 8

#define PACKET_ADDR 0x8000

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

unsigned char mac_addr[6];

void print_mac_addr(short io_addr) {
    for (int idx = 0; idx < 6; idx++) {
        unsigned char num = io_read_b(io_addr + idx);
        mac_addr[idx] = num;
        puth_n(num, 2);
        if (idx != 5) {
            puts_n(":");
        }
    }
}

void send_packet(unsigned short base_addr, char* text, int length) {
    io_write_w(base_addr + VIRTIO_QUEUE_SELECT_OFFSET, VIRTIO_QUEUE_TX);

    unsigned short queue_size = io_read_w(base_addr + VIRTIO_QUEUE_SIZE_OFFSET);
    puts_n("queue size: ");
    puth(queue_size, 4);

    struct virtio_net_hdr* packet = (struct virtio_net_hdr*)PACKET_ADDR;
    packet->flags = VIRTIO_NET_HDR_F_NEEDS_CSUM;
    packet->gso_size = 0;
    packet->hdr_len = sizeof(struct virtio_net_hdr);
    packet->csum_offset = 0;
    char* content = (char*)packet + sizeof(struct virtio_net_hdr);
    strcpy_n(content, "\xff\xff\xff\xff\xff\xff", 6);
    strcpy_n(content + 6, (char*)mac_addr, 6);
    strcpy_n(content + 12, "\x88\xb6", 2);
    strcpy_n(content + 14, text, length);

    struct virtq_desc* desc = (struct virtq_desc*)tx_queue;
    struct virtq_avail* avail = (struct virtq_avail*)(tx_queue + get_virtq_avail_offset(queue_size));

    desc->addr = (unsigned long long)packet;
    desc->len = sizeof(struct virtio_net_hdr) + 6 + 6 + 2 + length;
    desc->flags = 0;
    desc->next = 0;

    avail->flags = VIRTQ_AVAIL_F_NO_INTERRUPT;

    io_write_d(base_addr + VIRTIO_QUEUE_ADDR_OFFSET, ((unsigned long long)tx_queue) >> 12);

    io_write_b(base_addr + VIRTIO_DEVICE_STATUS_OFFSET, VIRTIO_STATUS_ACKNOWLEDGE | VIRTIO_STATUS_DRIVER | VIRTIO_STATUS_FEATURES_OK | VIRTIO_STATUS_DRIVER_OK);
    puts("the virtio driver is ready");

    avail->idx++;
    avail->ring[0] = 0;

    puts_n("packet content: ");
    puts(text);

    puts_n("packet length: ");
    puth(length, 8);

    io_write_b(base_addr + VIRTIO_NOTIFY_QUEUE_OFFSET, VIRTIO_QUEUE_TX);
    puts("notify queue update to the device");
}

void recv_packet(unsigned short base_addr) {
    io_write_w(base_addr + VIRTIO_QUEUE_SELECT_OFFSET, VIRTIO_QUEUE_RX);

    unsigned short queue_size = io_read_w(base_addr + VIRTIO_QUEUE_SIZE_OFFSET);
    puts_n("queue size: ");
    puth(queue_size, 4);

    struct virtq_desc* desc = (struct virtq_desc*)rx_queue;
    struct virtq_avail* avail = (struct virtq_avail*)(rx_queue + get_virtq_avail_offset(queue_size));

    desc->addr = (unsigned long long)rx_buffer;
    desc->len = RX_BUFFER_SIZE;
    desc->flags = 2;
    desc->next = 0;

    avail->flags = VIRTQ_AVAIL_F_NO_INTERRUPT;

    io_write_d(base_addr + VIRTIO_QUEUE_ADDR_OFFSET, ((unsigned long long)rx_queue) >> 12);

    io_write_b(base_addr + VIRTIO_DEVICE_STATUS_OFFSET, VIRTIO_STATUS_ACKNOWLEDGE | VIRTIO_STATUS_DRIVER | VIRTIO_STATUS_FEATURES_OK | VIRTIO_STATUS_DRIVER_OK);
    puts("the virtio driver is ready");

    avail->idx++;
    avail->ring[0] = 0;

    io_write_b(base_addr + VIRTIO_NOTIFY_QUEUE_OFFSET, VIRTIO_QUEUE_RX);
    puts("notify queue update to the device");

    struct virtq_used* used = (struct virtq_used*)(rx_queue + get_used_offset(queue_size));

    unsigned short prev  = used->idx;
    while (prev == used->idx) {
        volatile int i = 1000000;
        while (i--);
    }

    puts("received a packet");

    unsigned int id = used->ring[0].id;
    unsigned int len = used->ring[0].len - sizeof(struct virtio_net_hdr);

    puts("flag contents:");

    struct virtio_net_hdr* header = (struct virtio_net_hdr*)desc[id].addr;
    puts_n("| flags: ");
    puth(header->flags, 2);

    puts_n("| gso type: ");
    puth(header->gso_type, 2);

    puts_n("| header length: ");
    puth(header->hdr_len, 4);

    puts_n("| gso size: ");
    puth(header->gso_size, 4);

    puts_n("| csum start: ");
    puth(header->csum_start, 4);

    puts_n("| csum offset: ");
    puth(header->csum_offset, 4);

    puts_n("| content: ");
    char* text = ((char*)desc[id].addr) + sizeof(struct virtio_net_hdr);
    for (unsigned int idx = 0; idx < len; idx++) {
        puth_n(text[idx], 2);
    }
    puts("");

    puts_n("| content length: ");
    puth(len, 16);
}

void init_virtio_driver() {
    unsigned int config_addr = search_virtio_net();
    if (!config_addr) {
        puts("failed to find virtio.");
        return;
    }

    puts_n("config_addr: ");
    puth(config_addr, 8);

    unsigned short base_addr = read_pci_config_data(config_addr | 0x10) ^ 0x1;

    io_write_b(base_addr + VIRTIO_DEVICE_STATUS_OFFSET, VIRTIO_STATUS_ACKNOWLEDGE);
    io_write_b(base_addr + VIRTIO_DEVICE_STATUS_OFFSET, VIRTIO_STATUS_ACKNOWLEDGE | VIRTIO_STATUS_DRIVER);
    puts("acknowledged the device");

    io_write_d(base_addr + VIRTIO_DEVICE_FEATURES_OFFSET, VIRTIO_NET_F_CSUM | VIRTIO_NET_F_MAC);
    io_write_b(base_addr + VIRTIO_DEVICE_STATUS_OFFSET, VIRTIO_STATUS_ACKNOWLEDGE | VIRTIO_STATUS_DRIVER | VIRTIO_STATUS_FEATURES_OK);
    puts("register driver features");
    if (!(io_read_b(base_addr + VIRTIO_DEVICE_STATUS_OFFSET) & VIRTIO_STATUS_FEATURES_OK)) {
        puts("features required are not supported by the device");
        return;
    }

    puts_n("mac addr: ");
    print_mac_addr(base_addr + VIRTIO_MAC_OFFSET);
    puts("");

    send_packet(base_addr, "I love commit_creds(&init_cred).", 33);

    puts("a packet sent");

    recv_packet(base_addr);
}

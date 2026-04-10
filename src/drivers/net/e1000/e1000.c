#include "e1000.h"
#include <kernel/pci/pci.h>
#include <kernel/pci/device.h>
#include <kernel/pci/config.h>
#include <kernel/mem/meminclude.h>
#include <kernel/include/reqs.h>
#include <memory/main.h>
#include <string/string.h>
#include <kernel/communication/serial.h>
#include <theme/doccr.h>

static struct
{
    volatile u32 *mmio;
    u8 mac[6];

    e1000_rx_desc_t *rx_descs;
    u64 rx_descs_phys;
    u8 *rx_bufs[E1000_RX_COUNT];
    u64 rx_bufs_phys[E1000_RX_COUNT];
    u32 rx_tail;

    e1000_tx_desc_t *tx_descs;
    u64 tx_descs_phys;
    u8 *tx_bufs[E1000_TX_COUNT];
    u64 tx_bufs_phys[E1000_TX_COUNT];
    u32 tx_tail;

    int present;
} dev;

static inline u32 reg_read(u32 off)
{
    return dev.mmio[off >> 2];
}

static inline void reg_write(u32 off, u32 val)
{
    dev.mmio[off >> 2] = val;
}

static u16 eeprom_read(u8 addr)
{
    reg_write(E1000_EERD, 1u | ((u32)addr << 8));
    u32 v = 0;
    for (int i = 0; i < 100000; i++)
    {
        v = reg_read(E1000_EERD);
        if (v & (1u << 4)) break;
        if (v & (1u << 1)) break;
    }
    return (u16)(v >> 16);
}

static void read_mac(void)
{
    u16 w0 = eeprom_read(0);
    u16 w1 = eeprom_read(1);
    u16 w2 = eeprom_read(2);
    dev.mac[0] = w0 & 0xFF; dev.mac[1] = w0 >> 8;
    dev.mac[2] = w1 & 0xFF; dev.mac[3] = w1 >> 8;
    dev.mac[4] = w2 & 0xFF; dev.mac[5] = w2 >> 8;
}

static int rx_init(void)
{
    u64 hhdm = hhdm_request.response->offset;
    u64 pages = (sizeof(e1000_rx_desc_t) * E1000_RX_COUNT + 0xFFF) / 0x1000;

    dev.rx_descs_phys = physmem_alloc_to(pages);
    if (!dev.rx_descs_phys) return -1;

    dev.rx_descs = (e1000_rx_desc_t *)(dev.rx_descs_phys + hhdm);
    memset(dev.rx_descs, 0, sizeof(e1000_rx_desc_t) * E1000_RX_COUNT);

    for (int i = 0; i < E1000_RX_COUNT; i++)
    {
        dev.rx_bufs_phys[i] = physmem_alloc_to(1);
        if (!dev.rx_bufs_phys[i]) return -1;
        dev.rx_bufs[i] = (u8 *)(dev.rx_bufs_phys[i] + hhdm);
        dev.rx_descs[i].addr = dev.rx_bufs_phys[i];
    }

    reg_write(E1000_RDBAL, (u32)(dev.rx_descs_phys & 0xFFFFFFFF));
    reg_write(E1000_RDBAH, (u32)(dev.rx_descs_phys >> 32));
    reg_write(E1000_RDLEN, sizeof(e1000_rx_desc_t) * E1000_RX_COUNT);
    reg_write(E1000_RDH, 0);
    reg_write(E1000_RDT, E1000_RX_COUNT - 1);
    dev.rx_tail = 0;

    reg_write(E1000_RCTL, E1000_RCTL_EN | E1000_RCTL_BAM | E1000_RCTL_SECRC);
    return 0;
}

static int tx_init(void)
{
    u64 hhdm = hhdm_request.response->offset;
    u64 pages = (sizeof(e1000_tx_desc_t) * E1000_TX_COUNT + 0xFFF) / 0x1000;

    dev.tx_descs_phys = physmem_alloc_to(pages);
    if (!dev.tx_descs_phys) return -1;

    dev.tx_descs = (e1000_tx_desc_t *)(dev.tx_descs_phys + hhdm);
    memset(dev.tx_descs, 0, sizeof(e1000_tx_desc_t) * E1000_TX_COUNT);

    for (int i = 0; i < E1000_TX_COUNT; i++)
    {
        dev.tx_bufs_phys[i] = physmem_alloc_to(1);
        if (!dev.tx_bufs_phys[i]) return -1;
        dev.tx_bufs[i] = (u8 *)(dev.tx_bufs_phys[i] + hhdm);
        dev.tx_descs[i].addr = dev.tx_bufs_phys[i];
        dev.tx_descs[i].status = E1000_TXD_DD; // mark free
    }

    reg_write(E1000_TDBAL, (u32)(dev.tx_descs_phys & 0xFFFFFFFF));
    reg_write(E1000_TDBAH, (u32)(dev.tx_descs_phys >> 32));
    reg_write(E1000_TDLEN, sizeof(e1000_tx_desc_t) * E1000_TX_COUNT);
    reg_write(E1000_TDH, 0);
    reg_write(E1000_TDT, 0);

    dev.tx_tail = 0;

    reg_write(E1000_TCTL, E1000_TCTL_EN | E1000_TCTL_PSP | (0x0F << 4) | (0x40 << 12));
    reg_write(E1000_TIPG, 0x0060200A);

    return 0;
}

int e1000_init(void)
{
    memset(&dev, 0, sizeof(dev));

    if (!hhdm_request.response) {
        log("[E1000]", "no hhdm\n", error);
        return -1;
    }

    /*try the devices we know about */
    pci_device_t *pci = NULL;
    const u16 ids[] =  { E1000_82540EM, E1000_82545EM, E1000_82574L, 0};

    for (int i = 0; ids[i]; i++)
    {
        pci = pci_device_find_by_vendor(E1000_VENDOR, ids[i]);
        if (pci) break;
    }

    if (!pci)
    {
        log("[E1000]", "not found\n", warning);
        return -1;
    }


    u16 cmd = pci_config_read_word(pci->bus, pci->device, pci->function, 0x04);
    cmd |= (1u << 1) | (1u << 2);
    pci_config_write_word(pci->bus, pci->device, pci->function, 0x04, cmd);

    u32 bar0 = pci_config_read(pci->bus, pci->device, pci->function, 0x10);
    u64 mmio_phys = bar0 & ~0xFull;

    if ((bar0 & 0x6) == 0x4) mmio_phys |= (u64)pci_config_read(pci->bus, pci->device, pci->function, 0x14) << 32;

    /*maps 64KB for E1000 MMIO */
    map_region(hhdm_request.response,
               mmio_phys,
               hhdm_request.response->offset + mmio_phys,
               0x10000,
               KERNEL_FLAGS);
    /*now use the allocated space */
    dev.mmio = (volatile u32 *)(mmio_phys + hhdm_request.response->offset);

    /* reset */
    reg_write(E1000_CTRL, reg_read(E1000_CTRL) | E1000_CTRL_RST);
    for (volatile int i = 0; i < 200000; i++) __asm__ volatile("nop");
    while (reg_read(E1000_CTRL) & E1000_CTRL_RST);

    reg_write(E1000_CTRL, (reg_read(E1000_CTRL) | E1000_CTRL_SLU | E1000_CTRL_ASDE));
    reg_write(E1000_IMC, 0xFFFFFFFF);

    /* clear multicast table */
    for (int i = 0; i < 0x80; i++) reg_write(E1000_MTA + i * 4, 0);

    read_mac();

    u32 ral = dev.mac[0] | (dev.mac[1] << 8) | (dev.mac[2] << 16) | (dev.mac[3] << 24);
    u32 rah = dev.mac[4] | (dev.mac[5] << 8) | (1u << 31);
    reg_write(E1000_RAL, ral);
    reg_write(E1000_RAH, rah);

    if (rx_init() != 0 || tx_init() != 0) return -1;

    dev.present = 1;

    char mac_str[32];
    str_copy(mac_str, "MAC: ");

    for (int i = 0; i < 6; i++)
    {
        static const char hex[] = "0123456789ABCDEF";
        char b[4] = { hex[dev.mac[i] >> 4], hex[dev.mac[i] & 0xF], i < 5 ? ':' : '\n', '\0' };
        str_append(mac_str, b);
    }

    log("[E1000]", mac_str, success);

    return 0;
}

int e1000_send(const void *data, u16 len)
{
    if (!dev.present || !data || !len || len > E1000_BUF_SIZE) return -1;

    u32 tail = dev.tx_tail;

    /* wait for the slot to free up
     */
    for (
    	int i = 0; i < 10000 && !(dev.tx_descs[tail].status & E1000_TXD_DD); i++
    ) for (volatile int d = 0; d < 100; d++) __asm__ volatile("nop");

    if (!(dev.tx_descs[tail].status & E1000_TXD_DD)) return -1;

    memcpy(dev.tx_bufs[tail], data, len);

    dev.tx_descs[tail].length = len;
    dev.tx_descs[tail].status = 0;
    dev.tx_descs[tail].cmd = E1000_TXD_EOP | E1000_TXD_FCS | E1000_TXD_RS;
    dev.tx_descs[tail].cso = 0;
    dev.tx_descs[tail].css = 0;

    u32 next = (tail + 1) % E1000_TX_COUNT;

    reg_write(E1000_TDT, next);
    dev.tx_tail = next;
    return 0;
}
int e1000_recv(void *buf, u16 max_len)
{
    if (!dev.present || !buf) return 0;

    u32 head = dev.rx_tail;

    if (!(dev.rx_descs[head].status & E1000_RXD_DD)) return 0;

    u16 len = dev.rx_descs[head].length;
    if (len > max_len) len = max_len;

    memcpy(buf, dev.rx_bufs[head], len);

    dev.rx_descs[head].status = 0;
    reg_write(E1000_RDT, head);
    dev.rx_tail = (head + 1) % E1000_RX_COUNT;
    return len;
}

/* other kernel-functions can look if e1000 is available */
int e1000_present(void)
{
    return dev.present;
}

void e1000_get_mac(u8 mac[6])
{
	/* look until it finds a mac adress
	 */
    for (int i = 0; i < 6; i++) mac[i] = dev.mac[i];
}

/* network driver layer */
/*static net_driver_t e1000_driver =
{
    .init 		= e1000_init,
    .send 		= e1000_send,
    .recv 		= e1000_recv,
    .get_mac 	= e1000_get_mac
};*/
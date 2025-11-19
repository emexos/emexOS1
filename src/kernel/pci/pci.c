#include "pci.h"
#include <kernel/include/ports.h>
#include <klib/string/string.h>
#include <theme/stdclrs.h>

#define MAX_PCI_DEVICES 32

static pci_device_t devices[MAX_PCI_DEVICES];
static int device_count = 0;

u32 pci_read(u8 bus, u8 device, u8 function, u8 offset) {
    u32 address = (u32)((bus << 16) | (device << 11) |
                        (function << 8) | (offset & 0xFC) | 0x80000000);

    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

static void pci_scan_bus(u8 bus) {
    for (u8 device = 0; device < 32; device++) {
        u32 vendor = pci_read(bus, device, 0, 0);
        u16 vendor_id = vendor & 0xFFFF;
        u16 device_id = (vendor >> 16) & 0xFFFF;

        if (vendor_id != 0xFFFF && device_count < MAX_PCI_DEVICES) {
            devices[device_count].vendor_id = vendor_id;
            devices[device_count].device_id = device_id;
            devices[device_count].bus = bus;
            devices[device_count].device = device;
            devices[device_count].function = 0;
            device_count++;
        }
    }
}

void pci_init(void) {
    char buf[64];
    buf[0] = '\0'; // prevents it from random character drawing
// before pci_get_device_count();
// because in the kernel its used before this causes random character drawing


    print("[PCI] ", GFX_GRAY_70);

    device_count = 0;
    pci_scan_bus(0);

    print("found: ", GFX_ST_WHITE);
    str_append_uint(buf, pci_get_device_count());
    print(buf, GFX_ST_YELLOW);
    print(" device(s)\n", GFX_ST_WHITE);
}

int pci_get_device_count(void) {
    return device_count;
}

pci_device_t* pci_get_device(int index) {
    if (index >= 0 && index < device_count) {
        return &devices[index];
    }
    return NULL;
}

#include "pci.h"
#include "device.h"
#include "config.h"
#include "express.h"
#include <klib/string/string.h>
#include <theme/stdclrs.h>
#include <klib/graphics/theme.h>
#include <theme/doccr.h>

void pci_init(void) {
    char buf[64];
    buf[0] = '\0'; // prevents it from random character drawing
// before pci_get_device_count();
// because in the kernel its used before this causes random character drawing

    BOOTUP_PRINT("[PCI] ", GFX_GRAY_70);
    BOOTUP_PRINT("Init PCI/PCIe \n", GFX_ST_WHITE);

    pci_device_init();


    int count = pci_device_get_count();

    // count PCIe devices
    int pcie_count = 0;
    for (int i = 0; i < count; i++) {
        pci_device_t *dev = pci_device_get(i);
        if (dev && pcie_is_device(dev->bus, dev->device, dev->function)) {
            pcie_count++;
        }
    }
    BOOTUP_PRINT("[PCI] ", GFX_GRAY_70);
    BOOTUP_PRINT("found: ", GFX_ST_WHITE);
    str_append_uint(buf, pcie_count);
    BOOTUP_PRINT(buf, GFX_ST_YELLOW);
    BOOTUP_PRINT(" device(s)\n", GFX_ST_WHITE);

}

int pci_get_device_count(void) {
    return pci_device_get_count();
}

pci_device_t* pci_get_device(int index) {
    return pci_device_get(index);
}

#include "pci.h"
#include "device.h"
#include "config.h"
#include "express.h"
#include <string/string.h>
#include <theme/stdclrs.h>
#include <kernel/graph/theme.h>
#include <theme/doccr.h>

void pci_init(void) {
    char buf[64];
    buf[0] = '\0'; // prevents it from random character drawing
// before pci_get_device_count();
// because in the kernel its used before this causes random character drawing

    log("[PCI]", "Init PCI/PCIe", d);

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
    log("[PCI]", "found ", d);
    str_append_uint(buf, pcie_count);
    BOOTUP_PRINT(buf, white());
    BOOTUP_PRINT(" device(s)\n", white());

}

int pci_get_device_count(void) {
    return pci_device_get_count();
}

pci_device_t* pci_get_device(int index) {
    return pci_device_get(index);
}

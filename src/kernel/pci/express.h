#ifndef PCI_EXPRESS_H
#define PCI_EXPRESS_H

#include <types.h>

typedef enum {
    PCIE_SPEED_UNKNOWN = 0,
    PCIE_SPEED_2_5_GT = 1,   // gen 1
    PCIE_SPEED_5_GT = 2,     // gen 2
    PCIE_SPEED_8_GT = 3,     // gen 3
    PCIE_SPEED_16_GT = 4,    // gen 4
    PCIE_SPEED_32_GT = 5     // gen 5
} pcie_link_speed_t;

// PCIe detection and info functions
int pcie_is_device(u8 bus, u8 device, u8 function);
pcie_link_speed_t pcie_get_link_speed(u8 bus, u8 device, u8 function);
u8 pcie_get_link_width(u8 bus, u8 device, u8 function);
const char* pcie_get_speed_string(pcie_link_speed_t speed);

#endif

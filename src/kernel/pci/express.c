#include "express.h"
#include "config.h"

// PCIe capability IDs
#define PCIE_CAP_ID 0x10

static u8 pci_find_capability(u8 bus, u8 device, u8 function, u8 cap_id)
{
    u16 status = pci_config_read_word(bus, device, function, PCI_STATUS);

    // check if device has capability list
    if (!(status & 0x10)) {
        return 0;
    }
    u8 cap_ptr = pci_config_read_byte(bus, device, function, PCI_CAPABILITIES);
    while (cap_ptr != 0) {
        u8 id = pci_config_read_byte(bus, device, function, cap_ptr);
        if (id == cap_id) {
            return cap_ptr;
        }
        cap_ptr = pci_config_read_byte(bus, device, function, cap_ptr + 1);
    }

    return 0;
}

int pcie_is_device(u8 bus, u8 device, u8 function) {
    return pci_find_capability(bus, device, function, PCIE_CAP_ID) != 0;
}

pcie_link_speed_t pcie_get_link_speed(u8 bus, u8 device, u8 function)
{
    u8 cap_ptr = pci_find_capability(bus, device, function, PCIE_CAP_ID);
    if (cap_ptr == 0) {
        return PCIE_SPEED_UNKNOWN;
    }
    // read Link-Status-Regs (cap_ptr + 0x12)
    u16 link_status = pci_config_read_word(bus, device, function, cap_ptr + 0x12);
    u8 speed = link_status & 0x0F;

    switch(speed) {
        case 1: return PCIE_SPEED_2_5_GT;
        case 2: return PCIE_SPEED_5_GT;
        case 3: return PCIE_SPEED_8_GT;
        case 4: return PCIE_SPEED_16_GT;
        case 5: return PCIE_SPEED_32_GT;
        default: return PCIE_SPEED_UNKNOWN;
    }
}

u8 pcie_get_link_width(u8 bus, u8 device, u8 function) {
    u8 cap_ptr = pci_find_capability(bus, device, function, PCIE_CAP_ID);
    if (cap_ptr == 0) {
        return 0;
    }

    // read Link-Status-Regs (cap_ptr + 0x12)
    u16 link_status = pci_config_read_word(bus, device, function, cap_ptr + 0x12);
    return (link_status >> 4) & 0x3F;
}

const char* pcie_get_speed_string(pcie_link_speed_t speed) {
    switch(speed) {
        case PCIE_SPEED_2_5_GT: return "2.5 gt/s (Gen. 1)";
        case PCIE_SPEED_5_GT:   return "5.0 gt/s (Gen. 2)";
        case PCIE_SPEED_8_GT:   return "8.0 gt/s (Gen. 3)";
        case PCIE_SPEED_16_GT:  return "16.0 gt/s (Gen. 4)";
        case PCIE_SPEED_32_GT:  return "32.0 gt/s (Gen. 5)";
        default:                return "Unknown";
    }
}

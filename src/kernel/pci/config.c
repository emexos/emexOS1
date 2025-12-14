#include "config.h"
#include <kernel/include/ports.h>

u32 pci_config_read(u8 bus, u8 device, u8 function, u8 offset) {
    u32 address = (u32)((bus << 16) | (device << 11) |
                        (function << 8) | (offset & 0xFC) | 0x80000000);

    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

void pci_config_write(u8 bus, u8 device, u8 function, u8 offset, u32 value)
{
    u32 address = (u32)((bus << 16) | (device << 11) |
                        (function << 8) | (offset & 0xFC) | 0x80000000);

    outl(PCI_CONFIG_ADDRESS, address);
    outl(PCI_CONFIG_DATA, value);
}

u16 pci_config_read_word(u8 bus, u8 device, u8 function, u8 offset) {
    u32 value = pci_config_read(bus, device, function, offset & 0xFC);
    return (u16)((value >> ((offset & 2) * 8)) & 0xFFFF);
}

u8 pci_config_read_byte(u8 bus, u8 device, u8 function, u8 offset) {
    u32 value = pci_config_read(bus, device, function, offset & 0xFC);
    return (u8)((value >> ((offset & 3) * 8)) & 0xFF);
}

void pci_config_write_word(u8 bus, u8 device, u8 function, u8 offset, u16 value) {
    u32 old = pci_config_read(bus, device, function, offset & 0xFC);
    u32 shift = (offset & 2) * 8;
    u32 mask = 0xFFFF << shift;
    u32 new = (old & ~mask) | ((u32)value << shift);
    pci_config_write(bus, device, function, offset & 0xFC, new);
}

void pci_config_write_byte(u8 bus, u8 device, u8 function, u8 offset, u8 value) {
    u32 old = pci_config_read(bus, device, function, offset & 0xFC);
    u32 shift = (offset & 3) * 8;
    u32 mask = 0xFF << shift;
    u32 new = (old & ~mask) | ((u32)value << shift);
    pci_config_write(bus, device, function, offset & 0xFC, new);
}

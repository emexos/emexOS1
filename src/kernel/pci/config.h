#ifndef PCI_CONFIG_H
#define PCI_CONFIG_H

#include <types.h>


//list from OSDEV
#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC
#define PCI_VENDOR_ID       0x00
#define PCI_DEVICE_ID       0x02
#define PCI_COMMAND         0x04
#define PCI_STATUS          0x06
#define PCI_REVISION        0x08
#define PCI_PROG_IF         0x09
#define PCI_SUBCLASS        0x0A
#define PCI_CLASS           0x0B
#define PCI_CACHE_LINE      0x0C
#define PCI_LATENCY_TIMER   0x0D
#define PCI_HEADER_TYPE     0x0E
#define PCI_BIST            0x0F
#define PCI_BAR0            0x10
#define PCI_BAR1            0x14
#define PCI_BAR2            0x18
#define PCI_BAR3            0x1C
#define PCI_BAR4            0x20
#define PCI_BAR5            0x24
#define PCI_CARDBUS_CIS     0x28
#define PCI_SUBSYSTEM_VID   0x2C
#define PCI_SUBSYSTEM_ID    0x2E
#define PCI_ROM_ADDRESS     0x30
#define PCI_CAPABILITIES    0x34
#define PCI_INTERRUPT_LINE  0x3C
#define PCI_INTERRUPT_PIN   0x3D
#define PCI_MIN_GRANT       0x3E
#define PCI_MAX_LATENCY     0x3F
#define PCI_CLASS_REVISION  0x08

// ptp bridge spec.
#define PCI_PRIMARY_BUS     0x18
#define PCI_SECONDARY_BUS   0x19
#define PCI_SUBORDINATE_BUS 0x1A

// config space access functions
u32 pci_config_read(u8 bus, u8 device, u8 function, u8 offset);
void pci_config_write(u8 bus, u8 device, u8 function, u8 offset, u32 value);
u16 pci_config_read_word(u8 bus, u8 device, u8 function, u8 offset);
u8 pci_config_read_byte(u8 bus, u8 device, u8 function, u8 offset);
void pci_config_write_word(u8 bus, u8 device, u8 function, u8 offset, u16 value);
void pci_config_write_byte(u8 bus, u8 device, u8 function, u8 offset, u8 value);

#endif

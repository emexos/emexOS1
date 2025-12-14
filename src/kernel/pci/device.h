#ifndef PCI_DEVICE_H
#define PCI_DEVICE_H

#include <types.h>

// PCIe device Structure
typedef struct {
    u16 vendor_id;
    u16 device_id;
    u8 bus;
    u8 device;
    u8 function;
    u8 class_code;
    u8 subclass;
    u8 prog_if;
    u8 revision;
    u8 header_type;
    u8 multifunction;
    u32 bar0;
} pci_device_t;

// management functions
void pci_device_init(void);
void pci_device_scan_bus(u8 bus);
int pci_device_get_count(void);
pci_device_t* pci_device_get(int index);
pci_device_t* pci_device_find_by_class(u8 class_code, u8 subclass);
pci_device_t* pci_device_find_by_vendor(u16 vendor_id, u16 device_id);
const char* pci_device_get_class_name(pci_device_t *dev);

#endif

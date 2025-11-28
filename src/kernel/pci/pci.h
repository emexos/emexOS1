#ifndef PCI_H
#define PCI_H

#include <types.h>
#include "device.h"
#include "config.h"
#include "express.h"

// all ports
#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

/*//TODO:
// maybe move these ports to include/pci.h
// but idk if i should do that

// device info
typedef struct {
    u16 vendor_id;
    u16 device_id;
    u8 bus;
    u8 device;
    u8 function;
} pci_device_t;
*/
void pci_init(void);
int pci_get_device_count(void);
pci_device_t* pci_get_device(int index);

#endif

#include "device.h"
#include "config.h"
#include <string/string.h>

#define MAX_PCI_DEVICES 256

static pci_device_t devices[MAX_PCI_DEVICES];
static int device_count = 0;

static const char* pci_get_class_name(u8 class_code)
{
    switch(class_code) {
        case 0x00: return "Unclassified";
        case 0x01: return "Mass Storage";
        case 0x02: return "Network";
        case 0x03: return "Display";
        case 0x04: return "Multimedia";
        case 0x05: return "Memory";
        case 0x06: return "Bridge";
        case 0x07: return "Communication";
        case 0x08: return "System";
        case 0x09: return "Input";
        case 0x0A: return "Docking";
        case 0x0B: return "Processor";
        case 0x0C: return "Serial Bus";
        case 0x0D: return "Wireless";
        case 0x0E: return "I - I/O";
        case 0x0F: return "Satellite";
        case 0x10: return "Encryption";
        case 0x11: return "Data Acquisition";
        default: return "Unknown";
    }
}

void pci_device_scan_bus(u8 bus)
{
    for (u8 device = 0; device < 32; device++) {
        for (u8 function = 0; function < 8; function++) {
            u32 vendor_device = pci_config_read(bus, device, function, PCI_VENDOR_ID);
            u16 vendor_id = vendor_device & 0xFFFF;
            u16 device_id = (vendor_device >> 16) & 0xFFFF;

            if (vendor_id == 0xFFFF) {
                if (function == 0) break; // no device
                continue;
            }

            if (device_count >= MAX_PCI_DEVICES) return;

            pci_device_t *dev = &devices[device_count];
            dev->vendor_id = vendor_id;
            dev->device_id = device_id;
            dev->bus = bus;
            dev->device = device;
            dev->function = function;
            // read additional info
            u32 class_info = pci_config_read(bus, device, function, PCI_CLASS_REVISION);
            dev->class_code = (class_info >> 24) & 0xFF;
            dev->subclass = (class_info >> 16) & 0xFF;
            dev->prog_if = (class_info >> 8) & 0xFF;
            dev->revision = class_info & 0xFF;

            u32 header_info = pci_config_read(bus, device, function, PCI_HEADER_TYPE);
            dev->header_type = (header_info >> 16) & 0x7F;
            dev->multifunction = (header_info >> 16) & 0x80;
            // Read BAR0 for base addres
            dev->bar0 = pci_config_read(bus, device, function, PCI_BAR0); // BAR0 is very important for xhci
            device_count++;

            if (function == 0 && !(dev->multifunction)) {
                break;
            }
        }
    }
}

void pci_device_init(void)
{
    device_count = 0;
    pci_device_scan_bus(0); // primary

    // check form ultiple buses
    for (int i = 0; i < device_count; i++) {
        if (devices[i].class_code == 0x06 && devices[i].subclass == 0x04)
        {
            // ptp bridge found
            u32 bus_info = pci_config_read(devices[i].bus, devices[i].device,
                                          devices[i].function, PCI_SECONDARY_BUS);
            u8 secondary_bus = (bus_info >> 8) & 0xFF;
            if (secondary_bus > 0) {
                pci_device_scan_bus(secondary_bus);
            }
        }
    }
}

int pci_device_get_count(void) {
    return device_count;
}

pci_device_t* pci_device_get(int index) {
    if (index >= 0 && index < device_count) {
        return &devices[index];
    }
    return NULL;
}

pci_device_t* pci_device_find_by_class(u8 class_code, u8 subclass) {
    for (int i = 0; i < device_count; i++) {
        if (devices[i].class_code == class_code &&
            (subclass == 0xFF || devices[i].subclass == subclass)) {
            return &devices[i];
        }
    }
    return NULL;
}

pci_device_t* pci_device_find_by_vendor(u16 vendor_id, u16 device_id) {
    for (int i = 0; i < device_count; i++) {
        if (devices[i].vendor_id == vendor_id &&
            (device_id == 0xFFFF || devices[i].device_id == device_id)) {
            return &devices[i];
        }
    }
    return NULL;
}

const char* pci_device_get_class_name(pci_device_t *dev) {
    if (!dev) return "Invalid";
    return pci_get_class_name(dev->class_code);
}

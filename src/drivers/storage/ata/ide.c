// src/drivers/storage/ata/ide.c
#include <kernel/pci/pci.h>
#include <kernel/include/ports.h>
#include <string/string.h>
#include <theme/stdclrs.h>
#include <kernel/graph/theme.h>
#include <theme/doccr.h>

#define PCI_CLASS_MASS_STORAGE 0x01
#define PCI_SUBCLASS_IDE       0x01

typedef struct {
    u16 vendor_id;
    u16 device_id;
    u8 bus;
    u8 device;
    u8 function;
    u32 bar0;  // primary Command Block
    u32 bar1;  // primary Control Block
    u32 bar2;  // secondary Command Block
    u32 bar3;  // secondary Control Block
    u32 bar4;  // bus-master IDE
} pci_ide_controller_t;

static pci_ide_controller_t ide_controller;
static int ide_found = 0;


static u32 pci_read_config(u8 bus, u8 device, u8 function, u8 offset) {
    u32 address = (1 << 31) | (bus << 16) | (device << 11) |
                  (function << 8) | (offset & 0xFC);
    outl(0xCF8, address);
    return inl(0xCFC);
}

int pci_find_ide_controller(void) {
    BOOTUP_PRINT("[PCI-IDE] ", GFX_GRAY_70);
    BOOTUP_PRINT("Searching for IDE controller:\n", white());

    // Scan all PCI devices
    for (int bus = 0; bus < 8; bus++) {  // Reduziert
        for (int device = 0; device < 32; device++) {
            // check only function 0 first
            u32 vendor_device = pci_read_config(bus, device, 0, 0x00);

            if (vendor_device == 0xFFFFFFFF || vendor_device == 0) {
                continue;
            }

            // multifunction
            u32 header_type = pci_read_config(bus, device, 0, 0x0C);
            int max_func = (header_type & 0x00800000) ? 8 : 1;

            for (int function = 0; function < max_func; function++) {
                vendor_device = pci_read_config(bus, device, function, 0x00);

                if (vendor_device == 0xFFFFFFFF || vendor_device == 0) {
                    continue;
                }

                u32 class_code = pci_read_config(bus, device, function, 0x08);
                u8 class = (class_code >> 24) & 0xFF;
                u8 subclass = (class_code >> 16) & 0xFF;
                u8 prog_if = (class_code >> 8) & 0xFF;

                if (class == PCI_CLASS_MASS_STORAGE && subclass == PCI_SUBCLASS_IDE) {
                    ide_controller.vendor_id = vendor_device & 0xFFFF;
                    ide_controller.device_id = (vendor_device >> 16) & 0xFFFF;
                    ide_controller.bus = bus;
                    ide_controller.device = device;
                    ide_controller.function = function;

                    u32 cmd_reg = pci_read_config(bus, device, function, 0x04);
                    cmd_reg |= 0x05;
                    outl(0xCF8, (1 << 31) | (bus << 16) | (device << 11) | (function << 8) | 0x04);
                    outl(0xCFC, cmd_reg);

                    ide_controller.bar0 = pci_read_config(bus, device, function, 0x10);
                    ide_controller.bar1 = pci_read_config(bus, device, function, 0x14);
                    ide_controller.bar2 = pci_read_config(bus, device, function, 0x18);
                    ide_controller.bar3 = pci_read_config(bus, device, function, 0x1C);
                    ide_controller.bar4 = pci_read_config(bus, device, function, 0x20);

                    if (ide_controller.bar0 & 1) {
                        ide_controller.bar0 &= 0xFFFFFFFC;
                    }
                    if (ide_controller.bar1 & 1) {
                        ide_controller.bar1 &= 0xFFFFFFFC;
                    }
                    if (ide_controller.bar2 & 1) {
                        ide_controller.bar2 &= 0xFFFFFFFC;
                    }
                    if (ide_controller.bar3 & 1) {
                        ide_controller.bar3 &= 0xFFFFFFFC;
                    }
                    if (ide_controller.bar4 & 1) {
                        ide_controller.bar4 &= 0xFFFFFFFC;
                    }
                    // use legacy ports
                    if (ide_controller.bar0 == 0 || ide_controller.bar0 == 1) {
                        ide_controller.bar0 = 0x1F0;
                    }
                    if (ide_controller.bar1 == 0 || ide_controller.bar1 == 1) {
                        ide_controller.bar1 = 0x3F6;
                    }
                    if (ide_controller.bar2 == 0 || ide_controller.bar2 == 1) {
                        ide_controller.bar2 = 0x170;
                    }
                    if (ide_controller.bar3 == 0 || ide_controller.bar3 == 1) {
                        ide_controller.bar3 = 0x376;
                    }

                    char buf[128];
                    str_copy(buf, "[PCI-IDE] Found controller: ");

                    // vendor ID
                    char hex_chars[] = "0123456789ABCDEF";
                    char vendor_str[5];
                    vendor_str[0] = hex_chars[(ide_controller.vendor_id >> 12) & 0xF];
                    vendor_str[1] = hex_chars[(ide_controller.vendor_id >> 8) & 0xF];
                    vendor_str[2] = hex_chars[(ide_controller.vendor_id >> 4) & 0xF];
                    vendor_str[3] = hex_chars[ide_controller.vendor_id & 0xF];
                    vendor_str[4] = '\0';

                    str_append(buf, vendor_str);
                    str_append(buf, ":");

                    // device ID
                    char device_str[5];
                    device_str[0] = hex_chars[(ide_controller.device_id >> 12) & 0xF];
                    device_str[1] = hex_chars[(ide_controller.device_id >> 8) & 0xF];
                    device_str[2] = hex_chars[(ide_controller.device_id >> 4) & 0xF];
                    device_str[3] = hex_chars[ide_controller.device_id & 0xF];
                    device_str[4] = '\0';

                    str_append(buf, device_str);
                    str_append(buf, "\n");
                    BOOTUP_PRINT(buf, GFX_CYAN);

                    // print all BARs
                    str_copy(buf, "          BAR0 (Primary CMD):   0x");
                    str_append_uint(buf, ide_controller.bar0);
                    str_append(buf, "\n");
                    BOOTUP_PRINT(buf, white());

                    str_copy(buf, "          BAR1 (Primary CTRL):  0x");
                    str_append_uint(buf, ide_controller.bar1);
                    str_append(buf, "\n");
                    BOOTUP_PRINT(buf, white());

                    str_copy(buf, "          BAR2 (Secondary CMD): 0x");
                    str_append_uint(buf, ide_controller.bar2);
                    str_append(buf, "\n");
                    BOOTUP_PRINT(buf, white());

                    str_copy(buf, "          BAR3 (Secondary CTRL):0x");
                    str_append_uint(buf, ide_controller.bar3);
                    str_append(buf, "\n");
                    BOOTUP_PRINT(buf, white());

                    ide_found = 1;
                    return 1;
                }
            }
        }
    }

    BOOTUP_PRINT("[PCI-IDE] No IDE controller found\n", GFX_YELLOW);
    return 0;
}

int pci_ide_is_found(void) {
    return ide_found;
}

u16 pci_ide_get_primary_base(void) {
    return ide_found ? ide_controller.bar0 : 0x1F0;
}

u16 pci_ide_get_primary_ctrl(void) {
    return ide_found ? ide_controller.bar1 : 0x3F6;
}

u16 pci_ide_get_secondary_base(void) {
    return ide_found ? ide_controller.bar2 : 0x170;
}

u16 pci_ide_get_secondary_ctrl(void) {
    return ide_found ? ide_controller.bar3 : 0x376;
}

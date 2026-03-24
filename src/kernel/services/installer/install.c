#include "install.h"
int _ran = 0;

#include <kernel/inits/limine/cmd.h>
#include <kernel/file_systems/fat32/fat32.h>
#include <kernel/interface/partition.h>
#include <drivers/storage/ata/disk.h>
#include <kernel/include/reqs.h>
#include <kernel/communication/serial.h>
#include <kernel/graph/graphics.h>
#include <memory/main.h>
#include <limine/limine.h>

/*
 * only installs the kernel,
 * no other services
 */
const char *LIMINE_CONF_FILE =
    "timeout: 1\n" /* on slow hardware a timeout of 1 is always better */
    "\n"
    "/" KERNEL_BARENAME "\n"
    "   protocol: limine\n"
    "   kernel_path: boot():/boot/kernel_a.elf\n"
    "   module_path: boot():/boot/initrd.cpio\n"
    "\n"
    "   # updates"
    "	module_path: boot():/boot/kernel_a.elf"
    "	module_path: boot():/EFI/BOOT/BOOTX64.EFI"
    "	module_path: boot():/boot/limine/limine-bios.sys"
;

// print alias
static void ip(const char *msg, u32 col) {
    print(msg, col);
}
static void ip_int(u32 n, u32 col) {
    printInt((int)n, col);
}

static const char *mod_basename(const char *path) {
    const char *b = path;
    for (const char *p = path; *p; p++)
        if (*p == '/') b = p + 1;
    return b;
}
static struct limine_file *find_module(const char *fname) {
    if (!module_request.response) return NULL;
    struct limine_module_response *r = module_request.response;
    for (u64 i = 0; i < r->module_count; i++) {
        if (str_equals(mod_basename(r->modules[i]->path), fname))
            return r->modules[i];
    }
    return NULL;
}

int installer_run(void)
{
    if (_ran) return 0;
    _ran = 1;
    if (!cmd_is("install")) return 0;

    clear(bg());
    ip("\n  emexOS Installer\n", white());
    ip("\n\n", white());

    // ckecks if disk is available
    ATAdevice_t *dev = ATAget_device(0);
    if (!dev) {
        ip("[INSTALL] no disk found!\n", red());
        for (;;) __asm__ volatile("cli; hlt");
    }

    // if a disk was detected show it
    ip("[INSTALL] disk: ", white());
    ip(dev->model, cyan());
    ip("  ", white());
    ip_int((u32)(dev->sectors / 2048), white());
    ip(" MB\n", white());

    // partitioning and formating
    {
		ip("[INSTALL] writing MBR...\n", white());
		if (partition_format_disk_fat32() != 0) {
		    ip("[INSTALL] partition failed!\n", red());
		    for (;;) __asm__ volatile("cli; hlt");
		}

		partition_init();
		partition_info_t *part = partition_get_info(0);
		if (!part || !part->valid) {
		    ip("[INSTALL] no valid partition!\n", red());
		    for (;;) __asm__ volatile("cli; hlt");
		}

		ip("[INSTALL] FAT32 format at LBA:", white());
		ip_int(part->start_lba, white());
		ip("...\n", white());
		if (fat32_format_partition(part->start_lba, part->sector_count) != 0) {
		    ip("[INSTALL] FAT32 format failed...\n", red());
		    for (;;) __asm__ volatile("cli; hlt");
		}

    	ip("[INSTALL] init FAT32 driver...\n", white());
    	fat32_init(); // cuz in the kernel its initialized after the installer is running
    }


    ip("[INSTALL] creating dirs...\n", white());
    {
	    fat32_create_dir("/boot");
	    fat32_create_dir("/EFI");
	    fat32_create_dir("/EFI/BOOT");
	    fat32_create_dir("/boot/limine");
    }

    // install kernel
	{
		struct limine_file *kf = find_module("kernel_a.elf");
	    if (kf && kf->address && kf->size > 0) {
	        ip("[INSTALL] copying kernel (", white());
	        ip_int((u32)kf->size, white());
	        ip(" bytes)...\n", white());
	        fat32_write_file("/boot/kernel_a.elf", kf->address, (u32)kf->size);
	    } else {
	        ip("[INSTALL] kernel_a.elf module not found!\n", red());
	    }

	    //TODO:
	    // if cmdline has a update token
	    // then also install kernel B
	    //

	    //TODO:
	    // install (kernel) stub.elf
	}

    // installing ramdisk
	{
		struct limine_file *initrd = find_module("initrd.cpio");
	    if (initrd && initrd->address && initrd->size > 0) {
	        ip("[INSTALL] copying initrd (", white());
	        ip_int((u32)initrd->size, white());
	        ip(" bytes)...\n", white());

	        fat32_write_file("/boot/initrd.cpio", initrd->address, (u32)initrd->size);
	    } else {
	        ip("[INSTALL] initrd.cpio not found!\n", red());
	    }
	}

    // uefi bootloader
	{
		struct limine_file *efi = find_module("BOOTX64.EFI");
	    if (efi && efi->address && efi->size > 0) {
	        ip("[INSTALL] copying BOOTX64.EFI (", white());
	        ip_int((u32)efi->size, white());
	        ip(" bytes)...\n", white());
	        fat32_write_file("/EFI/BOOT/BOOTX64.EFI", efi->address, (u32)efi->size);
		} else {
		    ip("[INSTALL] BOOTX64.EFI not found!\n", red());
		}
	}

    // stage 2
    {
    	struct limine_file *bios_sys = find_module("limine-bios.sys");

		int r1 = fat32_write_file("/limine-bios.sys", bios_sys->address, (u32)bios_sys->size);
	    int r2 = fat32_write_file("/boot/limine/limine-bios.sys", bios_sys->address, (u32)bios_sys->size);

	    if (bios_sys && bios_sys->address && bios_sys->size > 0) {
	        ip("[INSTALL] copying limine-bios.sys (", white());
	        ip_int((u32)bios_sys->size, white());
	        ip(" bytes)...\n", white());


	        if (r1 < 0 && r2 < 0) {
				// if the bootloader install fails its definitly more dangerous than the kernel_<SLOT>.elf
				// cuz you can edit the entry in limine to load kernel_<SLOT>.elf
				// the kernel files are ALWAYS located in the isoroot
	            ip("[INSTALL] FATAL ERROR, BOOTLOADER WRITE FAILED\n", red());
	        } else {
	            ip("[INSTALL] limine-bios.sys ... (root=", white());
	            ip_int((u32)(r1 >= 0), white()); // root
	            ip(" /boot/limine=", white());
	            ip_int((u32)(r2 >= 0), white()); // third searching section
	            ip(")\n", white());
	        }
	    } else {
	        ip("[INSTALL] limine-bios.sys module not found!\n", red());
	    }
    }

    // limine.conf file
    {
	    ip("[INSTALL] writing limine.conf... (configuration file) \n", white());

		//root
	    fat32_write_file("/limine.conf", LIMINE_CONF_FILE, (u32)str_len(LIMINE_CONF_FILE));
		// third searching section
	    fat32_write_file("/boot/limine/limine.conf", LIMINE_CONF_FILE, (u32)str_len(LIMINE_CONF_FILE));
    }

    ip("\n[INSTALL] finished,  reboot now if something crashes ask the support for help \n\n", green());
    for (;;) __asm__ volatile("cli; hlt");
    return 1;
}
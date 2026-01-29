#include <kernel/include/assembly.h>
#include <kernel/include/reqs.h>

//theming
#include <kernel/graph/fm.h>
#include <kernel/include/logo.h>
#include <kernel/graph/theme.h>
#include <theme/doccr.h>
//#include <klib/graphics/font_manager.h>
//int init_boot_log = -1; // boot logs

// TODO:
// add a exact time for start and end, which cpu, how long it booted...

// pictures
#include <kernel/images/bmp.h>

// Drivers
#include <drivers/ps2/ps2.h>
//#include <drivers/usb/xhci.h>
//#include <drivers/ps2/keyboard/keyboard.h>

// CPU
#include <kernel/cpu/cpu.h>
#include <kernel/cpu/gdt.h>
#include <kernel/cpu/idt.h>
/*#include <kernel/cpu/isr.h>
#include <kernel/cpu/irq.h>
#include <kernel/cpu/timer.h>*/ // moved to /exception

// Memory
#include <memory/main.h>
#include <kernel/mem/meminclude.h>
//#include "mem_manager/virtmem.h" //not implemendet

//Console
#include <kernel/console/console.h>

//debug
#include <kernel/communication/serial.h>

//process
#include "exceptions/panic.h"

//exception/timer
#include <kernel/exceptions/timer.h>
#include <kernel/exceptions/panic.h>

//vFS & fs
#include <kernel/file_systems/vfs/vfs.h>
#include <kernel/file_systems/vfs/init.h>
//extern void fs_system_init(klime_t *klime);
//extern void fs_create_test_file(void);
#include <kernel/file_systems/fat32/fat32.h> // finally fat32!
// interface
#include <kernel/interface/partition.h>
#include <kernel/interface/mbr.h>
#include <config/disk.h>


// disk drivers
#include <kernel/module/module.h>
//extern void fs_register_mods(void);
#include <drivers/storage/ata/disk.h>


// limine modules
#include <kernel/modules/limine.h>
#include <kernel/inits/init.h>

//PCIe
#include <kernel/pci/pci.h>


// usermode stuff
//#include <kernel/syscalls/syscall.h>
//#include <kernel/userspace/userspace.h>
//extern void userspace_test_init(ulime_t *ulime);

void _start(void)
{
    { // Initializing Boot screen
        theme_init();
        setcontext(THEME_BOOTUP); // gets loaded over sbootup_theme until, sbootup == FLU
        sbootup_theme(THEME_STD);
        //sconsole_theme(THEME_FLU);
        spanic_theme(THEME_STD);

        //f_setcontext(FONT_8X8_DOS);  // already set as default

        // Temporaly before switchin to glime_t
        // emexOS start
        // Ensure that Limine base revision is supported and that we have a framebuffer
        if (framebuffer_request.response == NULL ||
            framebuffer_request.response->framebuffer_count < 1) {
                printf("no response");
            hcf(); // enable text mode
        }

        // Initialize framebuffer graphics
        struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];
        graphics_init(fb);
        printf("\ninit graphics, draw logo\n");
        //draw_logo();

        // main kernel
        printf("==============================================\n");
        printf("==                  emexOS                  ==\n");
        printf("==    the OS which does what you tell it    ==\n");
        printf("==============================================\n");

        fm_init();
        clear(bg());

        #if BOOTUP_VISUALS == 1
            print("BOOTUP_VISUALS == 1\n", white());
        #else
            print("BOOTUP_VISUALS == 0\n", white());
        #endif

        //actually not needed but maybe later (e.g. for testing themes)
        //draw_rect(10, 10, fb_width - 20, fb_height - 20, blue());

    }

    char buf[512]; //for all string operations

    klime_t *klime;

    { // MADE BY @TSARAKI (github)

        // Initialize mem
        physmem_init(memmap_request.response, hhdm_request.response);
        paging_init(hhdm_request.response);

        // kernel lifetime
        u64 phys_klime = map_region_alloc(hhdm_request.response, HEAP_START, HEAP_SIZE);
        klime_t *klime = klime_init((u64 *)HEAP_START, HEAP_SIZE);

        if (!framebuffer_request.response) {
            panic("Cant initialize glime limine response NULL");
        }

        if (framebuffer_request.response->framebuffer_count < 1) {
            panic("Cant initialize glime limine framebuffer_count 0");
        }

        u64 phys_glime = map_region_alloc(hhdm_request.response, GRAPHICS_START, GRAPHICS_SIZE);

        limine_framebuffer_t *fb = framebuffer_request.response->framebuffers[0];

        glime_response_t glres;
        glres.start_framebuffer = (u64 *)fb->address;
        glres.width  = (u64)fb->width;
        glres.height = (u64)fb->height;
        glres.pitch  = (u64)fb->pitch;

        glime_t *glime = glime_init(&glres, (u64 *)GRAPHICS_START, GRAPHICS_SIZE);

        u64 phys_ulime = map_region_alloc(hhdm_request.response, ULIME_START, ULIME_META_SIZE);
        ulime_t *ulime = ulime_init(hhdm_request.response, klime, glime, phys_ulime);
        if (!ulime) {
            BOOTUP_PRINTF("Erorr: ulime is not initialized");
            panic( "Erorr: ulime is not initialized");
        }

        // Initialize the CPU
        cpu_detect();
        gdt_init();
        idt_init();
        u32 freq = 1000;
        timer_init(freq);
        BOOTUP_PRINT_INT(freq, white());
        BOOTUP_PRINT(" 1ms tick)\n", white());
        timer_set_boot_time(); //for uptime command

        pci_init();
        //pci will get really useful with xhci/other usb

        fs_system_init(klime);

        ata_init();
        {
            // Initialize partition system
            int part_result = partition_init();

            if (part_result != 0 || partition_needs_format()) {
                BOOTUP_PRINT("[DISK] ", GFX_GRAY_70);

                #if OVERWRITEALL == 1
                    BOOTUP_PRINT("(OVERWRITEALL=1)...\n", yellow());
                    if
                        (partition_format_disk_fat32() == 0)
                    {
                        BOOTUP_PRINT("[DISK] ", GFX_GRAY_70);
                        BOOTUP_PRINT("Creating FAT32 filesystem...\n", white());
                        if
                            (fat32_format_partition(2048, ATAget_device(0)->sectors - 4096) == 0)
                        {
                            BOOTUP_PRINT("[DISK] ", GFX_GRAY_70);
                            BOOTUP_PRINT("Disk formatted successfully!\n", green());
                            partition_init();
                        }  else
                        {
                            BOOTUP_PRINT("[DISK] ", GFX_GRAY_70);
                            BOOTUP_PRINT("FAT32 format failed\n", red());
                        }
                    } else
                    {
                        BOOTUP_PRINT("[DISK] ", GFX_GRAY_70);
                        BOOTUP_PRINT("MBR creation failed\n", red());
                    }
                #else
                    BOOTUP_PRINT("Disk needs formatting\n", yellow());
                    BOOTUP_PRINT("set \"OVERWRITEALL=0\" to \"1\" in shared/config/disk.h\n", white());
                #endif
            }

            //fs_system_init(klime);

            BOOTUP_PRINT("[FAT32] ", GFX_GRAY_70);
            BOOTUP_PRINT("Mounting FAT32 filesystem...\n", white());
            fat32_init();
        }

        //BOOTUP_PRINT("\n", GFX_WHITE);
    }

    // Initialize Limine modules
    limine_modules_init();
    keymaps_load();
    logos_load();
    logo_init();
    draw_logo();
    cursor_x = 0;
    cursor_y = 10;

    //BOOTUP_PRINTF("\n");

    module_init();
    // Register driver modules
    BOOTUP_PRINT("[MOD] ", GFX_GRAY_70);
    BOOTUP_PRINT("Init regs: ", white());
    module_register(&console_module);
    module_register(&keyboard_module);
    int count = module_get_count();
    str_append_uint(buf, count);
    BOOTUP_PRINT(buf, yellow());
    BOOTUP_PRINT("\n", white());

    //scan through recent registered modules
    // this must happen AFTER module_register() calls
    fs_register_mods();

    //create test file in /tmp
    fs_create_test_file();

    buf[0] = '\0'; // clear buffer so it can be used again
    //    hcf();
    // delay(500); // for testing verbose/silent boot

    // write file
    /*int wf = fs_open("/tmp/log", O_CREAT | O_WRONLY);
    fs_write(wf, "test", 4);
    fs_close(wf);
    */

    if (init_boot_log >= 0) {
        fs_close(init_boot_log);
        init_boot_log = -1;
    }
    BOOTUP_PRINTF("\n");

    //hcf();

    user_config_init();

    console_init();
    keyboard_poll();

    //should not reach here
    //font_manager_set_context(FONT_CONTEXT_PANIC);
    #if USE_HCF == 1
        hcf();
    #else
        panic("USE_HCF; FAILED --> USING PANIC");
    #endif
};

#include <kernel/include/assembly.h>
#include <kernel/include/reqs.h>

//theming
#include <kernel/graph/fm.h>
#include <kernel/include/logo.h>
#include <kernel/graph/theme.h>
#include <math/math.h>
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
#if X64 == 1
    #include <kernel/arch/x64/gdt/gdt.h>
    #include <kernel/arch/x64/idt/idt.h>
    #include <kernel/arch/x64/exceptions/panic.h>
    #include <kernel/arch/x64/exceptions/timer.h>
#elif RISCV64 == 1
    #include <kernel/arch/riscv64/table/trap.h> // trap Table / exception Table
#elif AARCH64 == 1
    #include <kernel/arch/aarch64/exception/vectab.h> // vector table
#endif

// Memory
#include <memory/main.h>
#include <kernel/mem/meminclude.h>
//#include "mem_manager/virtmem.h" //not implemendet

//Console
#include <kernel/console/console.h>

//debug
#include <kernel/communication/serial.h>

//vFS & fs
#include <kernel/file_systems/vfs/vfs.h>
#include <kernel/file_systems/vfs/init.h>
//extern void fs_system_init(klime_t *klime);
//extern void fs_create_test_file(void);

#if ENABLE_FAT32
#include <kernel/file_systems/fat32/fat32.h> // finally fat32!
// interface
#include <kernel/interface/partition.h>
#include <kernel/interface/mbr.h>
#include <config/disk.h>

#endif

// disk drivers
#include <kernel/module/module.h>
//extern void fs_register_mods(void);
#if ENABLE_ATA
#include <drivers/storage/ata/disk.h>
#endif


// limine modules
#include <kernel/modules/limine.h>
#include <kernel/inits/init.h>

//PCIe
#include <kernel/pci/pci.h>


// usermode stuff
//#include <kernel/userspace/usermode.h>
//#include <kernel/syscalls/syscalls.h>
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
        cursor_x = 0;
        cursor_y = 0;
        font_scale = 1; // Setze scale für bootup

        #if BOOTUP_VISUALS == 1
            log("[BOOT]", "BOOTUP_VISUALS == 1\n", warning);
        #else
            log("[BOOT]", "BOOTUP_VISUALS == 0\n", warning);
        #endif

        BOOTUP_PRINT("\n\nWelcome to ", white());
        BOOTUP_PRINT("emexOS", cyan());
        BOOTUP_PRINT("!\n\n", white());

        //actually not needed but maybe later (e.g. for testing themes)
        //draw_rect(10, 10, fb_width - 20, fb_height - 20, blue());

    }

    char buf[512]; //for all string operations

    klime_t *klime = NULL;
    #if ENABLE_GLIME
        glime_t *glime = NULL;
    #endif
    #if ENABLE_ULIME
        ulime_t *ulime = NULL;
    #endif

    { // MADE BY @TSARAKI (github)

        cpu_detect();
        #if X64 == 1
            gdt_init();
            idt_init();
        #endif

        log("[MEM]", "Initializing memory management\n", d);
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

        #if ENABLE_GLIME
            u64 phys_glime = map_region_alloc(hhdm_request.response, GRAPHICS_START, GRAPHICS_SIZE);

            limine_framebuffer_t *fb = framebuffer_request.response->framebuffers[0];

            glime_response_t glres;
            glres.start_framebuffer = (u64 *)fb->address;
            glres.width  = (u64)fb->width;
            glres.height = (u64)fb->height;
            glres.pitch  = (u64)fb->pitch;

            glime_t *glime = glime_init(&glres, (u64 *)GRAPHICS_START, GRAPHICS_SIZE);
        #else
            log("[GLIME]", "skipped (hardware compatibility)\n", warning);
        #endif

        #if ENABLE_ULIME
            u64 phys_ulime = map_region_alloc(hhdm_request.response, ULIME_START, ULIME_META_SIZE);
        #if ENABLE_GLIME
            ulime = ulime_init(hhdm_request.response, klime, (void*)glime, phys_ulime);
        #else
            ulime = ulime_init(hhdm_request.response, klime, NULL, phys_ulime);
        #endif
        if (!ulime) {
            BOOTUP_PRINTF("Error: ulime is not initialized");
            panic("Error: ulime is not initialized");
        }
        #else
            log("[ULIME]", "skipped (hardware compatibility)\n", warning);
        #endif

        u32 freq = 1000;
        timer_init(freq);
        BOOTUP_PRINT_INT(freq, white());
        BOOTUP_PRINT(" 1ms tick)\n", white());
        timer_set_boot_time(); //for uptime command

        pci_init();
        //pci will get really useful with xhci/other usb

        fs_system_init(klime);

        //BOOTUP_PRINT("\n", GFX_WHITE);
    }
    #if ENABLE_ATA == 1
    ata_init();{
        // Initialize partition system
        int part_result = partition_init();

        if (part_result != 0 || partition_needs_format()) {
            //BOOTUP_PRINT("[DISK] ", GFX_GRAY_70);

            #if OVERWRITEALL == 1
                log("[DISK]", "(OVERWRITEALL=1)...\n", d);
                if
                    (partition_format_disk_fat32() == 0)
                {
                    log("[DISK]", "Creating FAT32 filesystem...\n", d);
                    if
                        (fat32_format_partition(2048, ATAget_device(0)->sectors - 4096) == 0)
                    {
                        log("[DISK]", "Disk formated successfully\n", success);
                        partition_init();
                    }  else
                    {
                        log("[DISK]", "FAT32 Format failed\n", error);
                    }
                } else
                {
                    log("[DISK]", "MBR creation failed\n", error);
                }
            #else
                log("[DISK]", "Disk needs formatting\n", warning);
                log("[DISK]", "set \"OVERWRITEALL=0\" to \"1\" in shared/config/disk.h\n", warning);
            #endif
        }
        #if ENABLE_FAT32 == 1
        log("[FAT32]", "mounting FAT32 file system\n", d);
        fat32_init();
        #endif
    }
    #else
        log("[ATA]", "skipped (hardware compatibility)\n", warning);
    #endif

    // Initialize Limine modules
    limine_modules_init(); {
        keymaps_load();
        logos_load();
    }
    logo_init();
    draw_logo();

    #if HARDWARE_SC == 1
        // let the cpu rest a small time
        for (volatile int i = 0; i < 1000000; i++) {
            __asm__ volatile("nop");
        }
    #endif

    module_init(); {
        // Register driver modules
        log("[MOD]", "Init regs:\n", d);
        module_register(&console_module);
        module_register(&keyboard_module);
        #if ENABLE_ATA
        module_register(&ata_module);
        #endif
        log("[MOD]", "found ", d);
        int count = module_get_count();
        str_append_uint(buf, count);
        BOOTUP_PRINT(buf, yellow());
        BOOTUP_PRINT(" module(s)\n", white());

    }

    { // Final things
        fs_register_mods();

        //create test file in /tmp
        fs_create_test_file();

        //buf[0] = '\0'; // clear buffer so it can be used again

        if (init_boot_log >= 0) {
            fs_close(init_boot_log);
            init_boot_log = -1;
        }

        uci();

        #if HARDWARE_SC == 1
            // let the cpu rest a small time
            for (volatile int i = 0; i < 1000000; i++) {
                __asm__ volatile("nop");
            }
        #endif
    }

    //hcf();

    /*#if ENABLE_ULIME
        if (ulime) {
            //usermodeinit(ulime);
        } else {
            hcf();
            panic("USE_HCF; FAILED --> USING PANIC");
            //console_init();
        }
    #else
        console_init();
    #endif*/

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

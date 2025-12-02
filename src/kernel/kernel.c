#include <kernel/include/assembly.h>
#include <kernel/include/reqs.h>
#include <kernel/console/console.h>
#include <kernel/include/logo.h>
#include <klib/graphics/theme.h>
#include <theme/doccr.h>
int init_boot_log = -1; // boot logs
// TODO:
// add a exact time for start and end, which cpu, how long it booted...

// Drivers
#include <drivers/ps2/ps2.h>
//#include <drivers/ps2/keyboard/keyboard.h>

// CPU
#include <kernel/cpu/cpu.h>
#include <kernel/cpu/gdt.h>
#include <kernel/cpu/idt.h>
/*#include <kernel/cpu/isr.h>
#include <kernel/cpu/irq.h>
#include <kernel/cpu/timer.h>*/ // moved to /exception

// Memory
#include <klib/memory/main.h>
#include <kernel/mem/meminclude.h>
//#include "mem_manager/virtmem.h" //not implemendet

//debug
#include <klib/debug/serial.h>

//process
#include "exceptions/panic.h"

//exception/timer
#include <kernel/exceptions/timer.h>
#include <kernel/exceptions/panic.h>

//vFS & fs
#include <kernel/file_systems/vfs/vfs.h>
//extern void fs_system_init(klime_t *klime);
//extern void fs_create_test_file(void);

// modules
#include <kernel/module/module.h>
//extern void fs_register_mods(void);

#include <kernel/pci/pci.h>

void _start(void)
{
    theme_init();
    setcontext(THEME_BOOTUP); // gets loaded over sbootup_theme until, sbootup == FLU
    sbootup_theme(THEME_STD);
    sconsole_theme(THEME_FLU);
    spanic_theme(THEME_STD);
    // Temporaly before switchin to glime_t
    // emexOS start
    // Ensure that Limine base revision is supported and that we have a framebuffer
    if (framebuffer_request.response == NULL ||
        framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    // Initialize framebuffer graphics
    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];
    graphics_init(fb);
    printf("\ninit graphics, draw logo\n");
    //draw_logo();

    // main kernel
    printf("==============================================\n");
    printf("==                                          ==\n");
    printf("==                  emexOS                  ==\n");
    printf("==                                          ==\n");
    printf("==============================================\n");

    #ifdef BOOTUP_VISUALS 1
        print("\BOOTUP_VISUALS == 1\n", white());
    #else
        print("\nBOOTUP_VISUALS == 0\n", white());
    #endif

    clear(bg());

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
        BOOTUP_PRINT("\n", GFX_WHITE);
        fs_system_init(klime);

        // Write only
        init_boot_log = fs_open("/tmp/log", O_CREAT | O_WRONLY);
        if (init_boot_log < 0) {
            panic("Cannot open /tmp/log");
        }
        //BOOTUP_PRINT("\n", GFX_WHITE);
    }

    BOOTUP_PRINT("[LOG] kernel bootup log:\n\n", GFX_WHITE);

    //actually not needed but maybe later (e.g. for testing themes)
    //draw_rect(10, 10, fb_width - 20, fb_height - 20, green());

    draw_logo();
    cursor_x = 0;
    cursor_y = 10;

    //BOOTUP_PRINTF("\n");

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


    BOOTUP_PRINT("[FONT] ", GFX_GRAY_70);
    BOOTUP_PRINT("scaling...\n", white());
    font_scale = 2;
    str_append_uint(buf, font_scale);
    BOOTUP_PRINT("[FONT] ", GFX_GRAY_70);
    BOOTUP_PRINT("scaled to: ", white());
    BOOTUP_PRINT(buf, white());
    BOOTUP_PRINT("\n", white());

    buf[0] = '\0'; // clear buffer so it can be used again

    BOOTUP_PRINT("[CONSOLE] ", GFX_GRAY_70);
    BOOTUP_PRINT("starting console...\n", white());
    //    hcf();
    // delay(500); // for testing verbose/silent boot
    clear(bg());
    // Initialize console and halt CPU

    //panic("test");
    //

    buf[0] = '\0';


    // write file
    /*int wf = fs_open("/tmp/log", O_CREAT | O_WRONLY);
    fs_write(wf, "test", 4);
    fs_close(wf);
    */

    BOOTUP_PRINTF("\n");
    BOOTUP_PRINTF("test printf\n");
    BOOTUP_PRINTF("test printf\n");

    if (init_boot_log >= 0) {
        fs_close(init_boot_log);
        init_boot_log = -1;
    }
    BOOTUP_PRINTF("\n");

    console_init();
    keyboard_poll();

    //should not reach here
    #ifdef USE_HCF
        hcf();
    #else
        panic("USE_HCF; FAILED --> USING PANIC");
    #endif
};

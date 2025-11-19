#include <kernel/include/assembly.h>
#include <kernel/include/reqs.h>
#include <kernel/console/console.h>
#include <kernel/include/logo.h>

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
#include "proc/process.h"
#include "proc/scheduler.h"

//exception/timer
#include <kernel/exceptions/timer.h>
#include <kernel/exceptions/panic.h>

// modules
#include <kernel/module/module.h>

#include <kernel/pci/pci.h>

void _start(void)
{
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
    printf("init graphics, draw logo\n");
    draw_logo();

    // main kernel
    printf("==============================================\n");
    printf("==                                          ==\n");
    printf("==                  emexOS                  ==\n");
    printf("==                                          ==\n");
    printf("==============================================\n");

    clear(BOOTSCREEN_BG_COLOR);

    char buf[128]; //for all string operations

    { // MADE BY @TSARAKI (github)
        // Initialize mem
        physmem_init(memmap_request.response, hhdm_request.response);
        u64 phys_start = paging_init(hhdm_request.response, HEAP_SIZE + GRAPHICS_SIZE);

        {
            // kernel lifetime
            map_region(hhdm_request.response, phys_start, HEAP_START, HEAP_SIZE);
            klime_t *klime = klime_init((u64 *)HEAP_START, HEAP_SIZE);

        }

        {

            if (!framebuffer_request.response) {
                panic("Cant initialize glime limine response NULL");
            }

            if (framebuffer_request.response->framebuffer_count < 1) {
                panic("Cant initialize glime limine framebuffer_count 0");
            }

            map_region(hhdm_request.response, phys_start + HEAP_SIZE, GRAPHICS_START, GRAPHICS_SIZE);

            limine_framebuffer_t *fb = framebuffer_request.response->framebuffers[0];

            glime_response_t glres;
            glres.start_framebuffer = (u64 *)fb->address;
            glres.width  = (u64)fb->width;
            glres.height = (u64)fb->height;
            glres.pitch  = (u64)fb->pitch;
            glres.bpp    = (u16)fb->bpp;

            glres.memory_model     = (u8)fb->memory_model;
            glres.red_mask_size    = (u8)fb->red_mask_size;
            glres.red_mask_shift   = (u8)fb->red_mask_shift;
            glres.green_mask_size  = (u8)fb->green_mask_size;
            glres.green_mask_shift = (u8)fb->green_mask_shift;
            glres.blue_mask_size   = (u8)fb->blue_mask_size;
            glres.blue_mask_shift  = (u8)fb->blue_mask_shift;

        }
    }

    //actually not needed but maybe later
    //draw_rect(10, 10, fb_width - 20, fb_height - 20, GFX_BG);

    draw_logo();
    cursor_x = 0;
    cursor_y = 10;

    printf("\n");

    // Initialize the CPU
    cpu_detect();
    gdt_init();
    idt_init();
    u32 freq = 1000;
    timer_init(freq);
    printInt(freq, GFX_ST_WHITE);
    print(" 1ms tick)\n", GFX_ST_WHITE);
    timer_set_boot_time(); //for uptime command

    pci_init();
    //pci will get really useful with xhci/other usb
    // Initialize process manager and scheduler
    process_init();
    sched_init();

    module_init();
    // Register driver modules
    print("[MOD] ", GFX_GRAY_70);
    print("Init regs: ", GFX_ST_WHITE);
    module_register(&console_module);
    module_register(&keyboard_module);
    int count = module_get_count();
    str_append_uint(buf, count);
    print(buf, GFX_ST_YELLOW);
    print("\n", GFX_ST_WHITE);

    buf[0] = '\0'; // clear buffer so it can be used again


    print("[FONT] ", GFX_GRAY_70);
    print("scaling...\n", GFX_ST_WHITE);
    font_scale = 2;
    str_append_uint(buf, font_scale);
    print("[FONT] ", GFX_GRAY_70);
    print("scaled to: ", GFX_ST_WHITE);
    print(buf, GFX_WHITE);
    print("\n", GFX_ST_WHITE);

    buf[0] = '\0'; // clear buffer so it can be used again

    print("[CONSOLE] ", GFX_GRAY_70);
    print("starting console...\n", GFX_ST_WHITE);
        hcf();
    clear(BOOTSCREEN_COLOR);
    // Initialize console and halt CPU

    //panic("test");

    printf("\n");
    printf("test printf\n");
    printf("test printf\n");
    console_init();
    keyboard_poll();

    //should not reach here
    #ifdef USE_HCF
        hcf();
    #else
        panic("USE_HCF; FAILED --> USING PANIC");
    #endif
};

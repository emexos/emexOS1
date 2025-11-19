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
    draw_logo();

    // main kernel
    printf("==============================================\n");
    printf("==                                          ==\n");
    printf("==                  emexOS                  ==\n");
    printf("==                                          ==\n");
    printf("==============================================\n");

    clear(BOOTSCREEN_BG_COLOR);
    
    char buf[256]; //for all string operations
    
    {
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
    //cursor_x = 10;
    //cursor_y = 10;

    // Initialize the CPU
    cpu_detect();
    print(" [CPU] Detected CPU\n", GFX_GREEN);
    gdt_init();
    print(" [GDT] init (Global Descriptor Table)\n", GFX_GREEN);
    idt_init();
    print(" [IDT] Init interrupts\n", GFX_GREEN);
    timer_init(1000);
    timer_set_boot_time(); //for uptime command
    //TODO:
    // its not exactly uptime because everything before imer_set_boot_time() doesnt get count
    // so we could set it to +50 milliseconds or something so its a bit more realistic i think...
    print(" [TIME] Init timer (1000Hz 1ms tick)\n", GFX_GREEN);
    print(" [TIME] started uptime now...\n", GFX_GREEN);

    putchar('\n', GFX_WHITE);

    pci_init();
    //char buf[64]; //its now at the top for every string operations
    str_copy(buf, " [PCI]: ");
    str_append_uint(buf, pci_get_device_count());
    str_append(buf, " device(s) found\n");
    print(buf, GFX_GREEN);
    //pci will get really useful with xhci/other usb
    //

    // Initialize process manager and scheduler
    // process_init();
    // print(" [PROC]  Process manager\n", GFX_GREEN);
    // sched_init();
    // print(" [SCHED] Scheduler\n", GFX_GREEN);

    putchar('\n', GFX_WHITE);
    module_init();
    print(" [MOD] Module system initialized\n", GFX_GREEN);

    // Register driver modules
    module_register(&console_module);
    print("test1\n", GFX_GREEN);

    module_register(&keyboard_module);
    print("test2\n", GFX_GREEN);

    // Initialize console and halt CPU

    //panic("test");

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

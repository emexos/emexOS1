#include <kernel/include/assembly.h>
#include <kernel/include/reqs.h>
#include <kernel/console/console.h>
#include <kernel/include/logo.h>

// Drivers
#include <drivers/ps2/ps2.h>
//#include <drivers/ps2/keyboard/keyboard.h>

// CPU
#include <kernel/cpu/gdt.h>
#include <kernel/cpu/idt.h>
/*#include <kernel/cpu/isr.h>
#include <kernel/cpu/irq.h>
#include <kernel/cpu/timer.h>*/ // moved to /exception

// Memory
#include <klib/memory/main.h>
#include <kernel/mem_manager/phys/physmem.h>
//#include "mem_manager/virtmem.h" //not implemendet

//process
#include "proc/process.h"
#include "proc/scheduler.h"

//exception/timer
#include <kernel/exceptions/timer.h>
#include <kernel/exceptions/panic.h>

// modules
#include <kernel/module/module.h>

// 16MB heap
#define HEAP_SIZE (8 * 1024 * 1024)
static u8 kernel_heap[HEAP_SIZE] __attribute__((aligned(16)));

void _start(void)
{
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
    // ==============================================
    // ==                                          ==
    // ==                  emexOS                  ==
    // ==                                          ==
    // ==============================================
    clear(BOOTSCREEN_COLOR);

    // Initialize heap and physical memory manager
    print("Initialized memory", TITLE_COLOR);
    mem_init(kernel_heap, HEAP_SIZE);
    char buf[128];
    str_copy(buf, "\nHeap: ");
    str_append_uint(buf, HEAP_SIZE / 1024);
    str_append(buf, " KB\n");
    print(buf, GFX_GRAY_50);

    putchar('\n', GFX_WHITE);

    physmem_init(256 * 1024 * 1024); // 256 MB
    print("Physical memory 256 MB\n", GFX_GREEN);

    //actually not needed but maybe later
    //draw_rect(10, 10, fb_width - 20, fb_height - 20, GFX_BG);

    draw_logo();
    //cursor_x = 10;
    //cursor_y = 10;

    // Initialize the CPU
    gdt_init();
    print("Initialized GDT (Global Descriptor Table)\n", GFX_GREEN);
    idt_init();
    print("Initialized interrupts\n", GFX_GREEN);
    timer_init(1000);
    print("Initialized timer (1000Hz 1ms tick)\n", GFX_GREEN);
    putchar('\n', GFX_WHITE);

    // Initialize process manager and scheduler
    putchar('\n', GFX_WHITE);
    process_init();
    print("Process manager\n", GFX_GREEN);
    sched_init();
    print("Scheduler\n", GFX_GREEN);

    putchar('\n', GFX_WHITE);
    module_init();
    print("Module system initialized\n", GFX_GREEN);

    // Register driver modules
    module_register(&console_module);
    print("test1\n", GFX_GREEN);

    module_register(&keyboard_module);
    print("test2\n", GFX_GREEN);

    // Show system info
    str_copy(buf, "Free Memory: ");
    str_append_uint(buf, (u32)(physmem_get_free() * 4) / 1024);
    str_append(buf, " MB\n");
    print(buf, GFX_CYAN);

    // Initialize console and halt CPU
    clear(BOOTSCREEN_COLOR);
    reset_cursor();
    draw_logo();
    console_init();
    keyboard_poll();

    //should not reach here
    #ifdef USE_HCF
        hcf();
    #else
        panic("USE_HCF; FAILED --> USING PANIC");
    #endif
};

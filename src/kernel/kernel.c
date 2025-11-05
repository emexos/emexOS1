// General
#include <kernel/include/assembly.h>
#include <kernel/console/console.h>
#include <kernel/include/logo.h>

// CPU
#include <kernel/cpu/gdt.h>
#include <kernel/cpu/idt.h>
#include <kernel/cpu/isr.h>
#include <kernel/cpu/irq.h>
#include <kernel/cpu/timer.h>

// Memory
#include <klib/memory/main.h>
#include <kernel/mem_manager/phys/physmem.h>
//#include "mem_manager/virtmem.h" //not implemendet

// Process
#include <kernel/proc/process.h>
#include <kernel/proc/scheduler.h>

// Drivers
#include <drivers/ps2/ps2.h>
#include <drivers/ps2/keyboard/keyboard.h>

// 16MB heap
#define HEAP_SIZE (8 * 1024 * 1024)
static u8 kernel_heap[HEAP_SIZE] __attribute__((aligned(16)));

// ==============================================
// ==                                          ==
// ==                  emexOS                  ==
// ==                                          ==
// ==============================================
void _start(void)
{
    // Ensure that Limine base revision is supported and that we have a framebuffer
    if (LIMINE_BASE_REVISION_SUPPORTED == 0) {
        hcf();
    }
    if (framebuffer_request.response == NULL ||
        framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    // Initialize framebuffer graphics
    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];
    graphics_init(fb);
    draw_logo();

    // init memory
    print("Memory", TITLE_COLOR);
    mem_init(kernel_heap, HEAP_SIZE);

    //actually not needed but maybe later
    //draw_rect(10, 10, fb_width - 20, fb_height - 20, GFX_BG);

    draw_logo();
    cursor_x = 20;
    cursor_y = 10;

    char buf[128];
    str_copy(buf, "\nHeap: ");
    str_append_uint(buf, HEAP_SIZE / 1024);
    str_append(buf, " KB\n");
    print(buf, GFX_GRAY_50);
    putchar('\n', GFX_WHITE);

    //clear(GFX_BG);
    draw_logo();

    // CPU
    gdt_init();
    print("GDT (Global Descriptor Table)\n", GFX_GREEN);
    idt_init();
    print("IDT IRQ ISR\n", GFX_GREEN);
    timer_init(1000); // 1000 Hz = 1ms ticks
    print("Timer initialized 1000Hz 1mstick\n", GFX_GREEN);
    putchar('\n', GFX_WHITE);

    // MEM
    physmem_init(256 * 1024 * 1024); // 256 MB
    print("Physical memory 256 MB\n", GFX_GREEN);
    putchar('\n', GFX_WHITE);


    // Process manager and scheduler
    process_init();
    print("Process manager\n", GFX_GREEN);
    sched_init();
    print("Scheduler\n", GFX_GREEN);

    putchar('\n', GFX_WHITE);
    putchar('\n', GFX_WHITE);

    // Initialize PCI
    //print("Hardware Detection\n", TITLE_COLOR);

    //pci_init();
    //print("PCI/PCIe bus\n", GFX_GREEN);

    //putchar('\n', GFX_WHITE);

    // Show system info
    str_copy(buf, "Free Memory: ");
    str_append_uint(buf, (u32)(physmem_get_free() * 4) / 1024);
    str_append(buf, " MB\n");
    print(buf, GFX_CYAN);

    // Start up the console and halt CPU
    clear(BOOTSCREEN_COLOR);
    reset_cursor();
    draw_logo();
    console_init();
    keyboard_poll();
    hcf();
};

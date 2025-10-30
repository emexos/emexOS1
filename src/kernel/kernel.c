#include "include/assembly.h"
#include "../libs/graphics/graphics.h"
//#include "../libs/graphics/standard/screen.h"
//#include "../libs/graphics/draw.h"
#include "../libs/graphics/theme/stdclrs.h"
#include "../libs/string/string.h"
#include "../libs/print/print.h"

#include "../libs/memory/main.h"
#include "../drivers/ps2/ps2.h"
//#include "../drivers/pci/pci.h"
/*#include "../drivers/usb/usb.h"
#include "../drivers/usb/xhci.h"
#include "../drivers/usb/usb_keyboard.h"*/
#include "console/console.h"
#include "../../shared/theme/doccr.h"

#include "include/logo.h"

#include "cpu/gdt.h"
#include "cpu/idt.h"
#include "cpu/isr.h"
#include "cpu/irq.h"
#include "cpu/timer.h"

#include "mem_manager/physmem.h"
//#include "mem_manager/virtmem.h" //not implemendet

#include "proc/process.h"
#include "proc/scheduler.h"

#include "../drivers/ps2/keyboard/keyboard.h"

// 16MB heap
#define HEAP_SIZE (8 * 1024 * 1024)
static u8 kernel_heap[HEAP_SIZE] __attribute__((aligned(16)));

void main(void)
{
    // emexOS

    //delay(30);

    clear(BOOTSCREEN_COLOR);

    // init memory
    print("Memory\n", TITLE_COLOR);
    mem_init(kernel_heap, HEAP_SIZE);

    //actually not needed but maybe later
    //draw_rect(10, 10, fb_width - 20, fb_height - 20, GFX_BG);

    draw_logo();

    cursor_x = 20;
    cursor_y = 10;

    delay(10);

    char buf[128];
    str_copy(buf, "Heap: ");
    str_append_uint(buf, HEAP_SIZE / 1024);
    str_append(buf, " KB\n");
    print(buf, GFX_GRAY_50);

    putchar('\n', GFX_WHITE);

    // run mem tests
    //mem_test();

    putchar('\n', GFX_WHITE);

    //clear(GFX_BG);
    draw_logo();

    // draw test
    /*
    rint("graphics Test", GFX_YELLOW);
    draw_rect(50, 350, 100, 60, GFX_RED);
    draw_circle(200, 380, 30, GFX_YELLOW);
    draw_line(300, 360, 400, 400, GFX_PURPLE);
    */

    // CPU
    gdt_init();
    print("GDT (Global Descriptor Table)\n", GFX_GREEN);
    idt_init();
    print("IDT IRQ ISR\n", GFX_GREEN);
    timer_init(1000); // 1000 Hz = 1ms ticks
    print("Timer initialized 1000Hz 1mstick\n", GFX_GREEN);

    delay(2);

    putchar('\n', GFX_WHITE);

    // MEM
    physmem_init(256 * 1024 * 1024); // 256 MB
    print("Physical memory 256 MB\n", GFX_GREEN);

    delay(10);

    putchar('\n', GFX_WHITE);


    process_init();
    print("Process manager\n", GFX_GREEN);
    scheduler_init();
    print("Scheduler\n", GFX_GREEN);

    putchar('\n', GFX_WHITE);
    putchar('\n', GFX_WHITE);

    // Initialize PCI
    print("Hardware Detection\n", TITLE_COLOR);

    //pci_init();
    print("PCI/PCIe bus\n", GFX_GREEN);

    putchar('\n', GFX_WHITE);

    // Show system info
    str_copy(buf, "Free Memory: ");
    str_append_uint(buf, (u32)(physmem_get_free_pages() * 4) / 1024);
    str_append(buf, " MB\n");
    print(buf, GFX_CYAN);

    // USB which is not implemented
    //print("\n[USB Initialization]\n", GFX_CYAN);
     //xhci_init();


    delay(10);

    clear(BOOTSCREEN_COLOR);
    reset_cursor();

    draw_logo();

    console_init();

    keyboard_poll();

    hcf();
}

void _start(void)
{
    // start os
    //

    if (LIMINE_BASE_REVISION_SUPPORTED == 0) {
        hcf();
    }

    if (framebuffer_request.response == NULL ||
        framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];

    graphics_init(fb)
        // should draw a rect
    ;

    draw_logo();

    mem_init(kernel_heap, HEAP_SIZE);

    main();
    // hcf(); cannot ever reach this
};

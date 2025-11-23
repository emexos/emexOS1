#include <kernel/include/assembly.h>
#include <kernel/include/reqs.h>
#include <kernel/console/console.h>
#include <kernel/include/logo.h>
#include <klib/graphics/theme.h>
#include <theme/doccr.h>

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

// modules
#include <kernel/module/module.h>

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
    draw_logo();

    // main kernel
    printf("==============================================\n");
    printf("==                                          ==\n");
    printf("==                  emexOS                  ==\n");
    printf("==                                          ==\n");
    printf("==============================================\n");

    //delay(500);
    clear(bg());

    char buf[128]; //for all string operations

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


        {
            //Should be the initd or getty or first proccess

            u8 *wsname = (u8 *)"root\0";
            int err1 = gworkspace_init(glime, wsname, 0);
            if (err1) {
                BOOTUP_PRINTF("workspace root is not initialized");
                panic( "workspace root is not initialized");
            }

            u8 *ssname = (u8 *)"0\0";
            gsession_t *s0 = gsession_init(glime, ssname, 400);
            if (!s0) {
                BOOTUP_PRINTF("session 0 is not initialized");
                panic( "session 0 is not initialized");
            }

            int err2 = gsession_attach(glime, s0, wsname);
            if (err2) {
                BOOTUP_PRINTF("session 0 is not attached");
                panic( "session 0 is not attached");
            }

            #if BOOTUP_VISUALS == 1
                gsession_clear(s0, 0x00000000);
            #else
                gsession_clear(s0, 0x00808080); // light gray
            #endif

            #if BOOTUP_VISUALS == 1
                u8 *sfst = (u8 *)" \0";
            #else
                u8 *sfst = (u8 *)"fst\0";
            #endif
            gsession_put_at_string_dummy(s0, sfst, 0, 0, 0x00FFFFFF);

            gworkspace_t *w0 = gworkspace_get_name(glime, wsname);
            if (!w0) {
                BOOTUP_PRINTF("workspace root is not found");
                panic( "workspace root is not found");
            }

            // int err3 = gsession_detach(w0, s0);
            // if (err3) {
            //     BOOTUP_PRINTF("session 0 is not detached");
            //     panic( "session 0 is not detached");
            // }

        }

        glime_commit(glime);

        BOOTUP_PRINTF("\n\n");

        u64 phys_ulime = map_region_alloc(hhdm_request.response, ULIME_START, ULIME_META_SIZE);
        ulime_t *ulime = ulime_init(hhdm_request.response, klime, glime, phys_ulime);
        if (!ulime) {
            BOOTUP_PRINTF("Erorr: ulime is not initialized");
            panic( "Erorr: ulime is not initialized");
        }

        ulime_init_syscalls(ulime);

        u8 *procname = (u8 *)"fstd\0";
        ulime_proc_t *p1 = ulime_proc_create(ulime, procname, 0x400000);
        if (p1) {
            int errp1 = ulime_proc_mmap(ulime, p1);
            if (errp1) {
                BOOTUP_PRINTF("Erorr: p1 is not mmaped");
                panic( "Erorr: p1 is not mmaped");
            }
        }

        ulime_proc_test_mem(p1);

        ulime_proc_list(ulime);

        int errpk = ulime_proc_kill(ulime, p1->pid);
        if (errpk) {
            BOOTUP_PRINTF("Erorr: p1 is not killed");
            panic( "Erorr: p1 is not killed");
        }

        ulime_proc_list(ulime);
    }

    //actually not needed but maybe later (e.g. for testing themes)
    //draw_rect(10, 10, fb_width - 20, fb_height - 20, green());

    draw_logo();
    cursor_x = 0;
    cursor_y = 10;

    BOOTUP_PRINTF("\n");

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

    BOOTUP_PRINTF("\n");
    BOOTUP_PRINTF("test printf\n");
    BOOTUP_PRINTF("test printf\n");
    console_init();
    keyboard_poll();

    //should not reach here
    #ifdef USE_HCF
        hcf();
    #else
        panic("USE_HCF; FAILED --> USING PANIC");
    #endif
};

#ifndef REQS_H
#define REQS_H

#include <limine/limine.h>

// Limine requests
extern volatile struct limine_framebuffer_request framebuffer_request;
extern volatile struct limine_memmap_request memmap_request;
extern volatile struct limine_hhdm_request hhdm_request;
extern volatile struct limine_module_request module_request; // for logo
extern volatile struct limine_executable_cmdline_request cmdline_request;
extern volatile struct limine_rsdp_request rsdp_request;

#endif

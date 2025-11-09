#include "reqs.h"
#include <types.h>
#include <limine/limine.h>

// limine reqs
__attribute__((used, section(".requests")))
volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(3);

// framebuffer
__attribute__((used, section(".requests")))
volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0,
    .response = NULL
};


//not used now
// mmap req
__attribute__((used, section(".requests")))
volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 0
};

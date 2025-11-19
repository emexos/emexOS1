#include "slab.h"
#include <klib/debug/serial.h>
#include <kernel/exceptions/panic.h>
#include <klib/memory/main.h>

void slab_init(slab_allocator_t *ptr_slab, u64 *ptr_slab_meta, u64 *ptr_slab_data) {  
  if (!ptr_slab) {
      panic("\nInvalid ptr slab\n");
  }

  if (!ptr_slab_meta) {
      panic("\nInvalid ptr slab meta\n");
  }

  if (!ptr_slab_data) {
      panic("\nInvalid ptr slab data\n");
  }

  memset(ptr_slab, 0, sizeof(slab_allocator_t));

}

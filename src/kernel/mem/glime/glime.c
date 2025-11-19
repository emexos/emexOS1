#include "glime.h"
#include <kernel/exceptions/panic.h>
#include "../heap/heap.h"

glime_t *glime_init(glime_response_t gr, u64 *ptr, u64 size) {
    if (!ptr || size < sizeof(heap_block_t)) {
        panic("ERROR Invlalid glime initializing");
    }

    glime_t *glime = (glime_t *)ptr;

    glime->glres = gr;
    glime->cursor_x = 0; 
    glime->cursor_y = 20; 
}

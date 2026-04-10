#include "memlog.h"

/*
 * prints the memory map start and hhdm offset
 */
void memlog_print_map(void)
{
    if (memmap_request.response)
    {
        log("[MEM]", "memmap start:\n", d);

        for (size_t i = 0; i < memmap_request.response->entry_count; i++)
        {
            struct limine_memmap_entry *e = memmap_request.response->entries[i];
            char buf[128];
            char tmp[32];
            // convert numbers to hex manually
            str_copy(buf, "Type: ");
            str_append_uint(buf, e->type);
            str_append(buf, ", Base: 0x");
            str_from_hex(tmp, e->base);
            str_append(buf, tmp);
            str_append(buf, ", Length: 0x");
            str_from_hex(tmp, e->length);
            str_append(buf, tmp);
            str_append(buf, "\n");
            log("[MEM]", buf, d);
        }

        log("[MEM]", ";\n", d);
    } else
    {
        log("[MEM]", "NOTHING FOUND\n", error);
    }

    if (hhdm_request.response)
    {
        char buf[64];
        char tmp[32];
        str_copy(buf, "HHDM Offset: 0x");
        str_from_hex(tmp, hhdm_request.response->offset);
        str_append(buf, tmp);
        str_append(buf, "\n");
        log("[MEM]", buf, d);
    }
}
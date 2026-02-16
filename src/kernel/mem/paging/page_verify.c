#include "paging.h"
#include <limine/limine.h>

#include "../mem.h"
#include "../phys/physmem.h"
#include <kernel/arch/x86_64/exceptions/panic.h>
#include <memory/main.h>

#include <string/string.h>
#include <theme/stdclrs.h>
#include <kernel/communication/serial.h>


int verify_page_permissions(limine_hhdm_response_t *hpr, u64 virtual_addr, const char *name) {
    u64 pml4_index = (virtual_addr >> 39) & 0x1FF;
    u64 pdp_index  = (virtual_addr >> 30) & 0x1FF;
    u64 pd_index   = (virtual_addr >> 21) & 0x1FF;
    u64 pt_index   = (virtual_addr >> 12) & 0x1FF;

    printf("[PAGE-VERIFY] Checking %s at 0x%lX:\n", name, virtual_addr);

    if (!(kernel_pml4->entries[pml4_index] & PTE_PRESENT)) {
        printf("  ERROR: PML4 not present\n");
        return 0;
    }
    printf("  PML4[%lu]: Present=%d User=%d\n",
           pml4_index,
           (kernel_pml4->entries[pml4_index] & PTE_PRESENT) ? 1 : 0,
           (kernel_pml4->entries[pml4_index] & PTE_USER) ? 1 : 0);

    u64 pdpt_phys = kernel_pml4->entries[pml4_index] & 0x000FFFFFFFFFF000;
    page_table_t* pdpt = (page_table_t*)(pdpt_phys + hpr->offset);

    if (!(pdpt->entries[pdp_index] & PTE_PRESENT)) {
        printf("  ERROR: PDPT not present\n");
        return 0;
    }
    printf("  PDPT[%lu]: Present=%d User=%d\n",
           pdp_index,
           (pdpt->entries[pdp_index] & PTE_PRESENT) ? 1 : 0,
           (pdpt->entries[pdp_index] & PTE_USER) ? 1 : 0);

    u64 pd_phys = pdpt->entries[pdp_index] & 0x000FFFFFFFFFF000;
    page_table_t* pd = (page_table_t*)(pd_phys + hpr->offset);

    if (!(pd->entries[pd_index] & PTE_PRESENT)) {
        printf("  ERROR: PD not present\n");
        return 0;
    }
    printf("  PD[%lu]: Present=%d User=%d\n",
           pd_index,
           (pd->entries[pd_index] & PTE_PRESENT) ? 1 : 0,
           (pd->entries[pd_index] & PTE_USER) ? 1 : 0);

    u64 pt_phys = pd->entries[pd_index] & 0x000FFFFFFFFFF000;
    page_table_t* pt = (page_table_t*)(pt_phys + hpr->offset);

    if (!(pt->entries[pt_index] & PTE_PRESENT)) {
        printf("  ERROR: PT not present\n");
        return 0;
    }

    u64 pte = pt->entries[pt_index];
    printf("  PT[%lu]: Present=%d Writable=%d User=%d NX=%d\n",
           pt_index,
           (pte & PTE_PRESENT) ? 1 : 0,
           (pte & PTE_WRITABLE) ? 1 : 0,
           (pte & PTE_USER) ? 1 : 0,
           (pte & PTE_NX) ? 1 : 0);

    // check if all levels have USER bit for user pages
    if (!(pte & PTE_USER)) {
        printf("  ERROR: Page not user-accessible!\n");
        return 0;
    }

    printf("  OK: Page is user-accessible\n");
    return 1;
}

#include <stdlib.h>
#include <stdio.h>
#include "page_table.h"

int page_table_init(PageTable *pt, uint32_t page_count)
{
    uint32_t i;

    pt->pte = calloc(page_count, sizeof(PageTableEntry));
    if (pt->pte == NULL)
        return 0;

    pt->page_count = page_count;

    for (i = 0; i < page_count; ++i) {
        pt->pte[i].present = 0U;
        pt->pte[i].pfn = 0U;
    }

    return 1;
}

void page_table_destroy(PageTable *pt)
{
    free(pt->pte);
    pt->pte = NULL;
    pt->page_count = 0U;
}

int page_table_lookup(const PageTable *pt,
                      uint32_t vpn,
                      uint32_t *pfn_out)
{

    if (vpn >= pt->page_count)
    {
        return 0;
    }
    if (pt->pte[vpn].present == 0U)
    {
        return 0;
    }
    *pfn_out = pt->pte[vpn].pfn;

    return 1;
}

int page_table_map(PageTable *pt,
                   uint32_t vpn,
                   uint32_t pfn)
{

    if (vpn >= pt->page_count)
    {
        return 0;
    }

    pt->pte[vpn].pfn = pfn;
    pt->pte[vpn].present = 1U;

    return 1;
}

int page_table_unmap(PageTable *pt,
                     uint32_t vpn)
{
    
    if (vpn >= pt->page_count)
    {
        return 0;
    }
    pt->pte[vpn].present = 0U;

    return 1;
}
void page_table_dump(
    const PageTable *pt,
    uint32_t pid
)
{
    uint32_t vpn;

    printf(
        "\n========== PAGE TABLE : PID %u ==========\n",
        pid
    );

    printf("VPN       Present    PFN\n");
    printf("-----------------------------------------\n");

    for (vpn = 0U; vpn < pt->page_count; vpn++)
    {
        if (pt->pte[vpn].present)
        {
            printf(
                "0x%06X  %u          0x%06X\n",
                vpn,
                pt->pte[vpn].present,
                pt->pte[vpn].pfn
            );
        }
    }
}
#include "tlb.h"

void tlb_init(TLB *tlb)
{
    uint32_t i;
    for (i = 0; i < TLB_ENTRIES; ++i) {
        tlb->entry[i].valid = 0U;
        tlb->entry[i].pid   = 0U;
        tlb->entry[i].vpn   = 0U;
        tlb->entry[i].pfn   = 0U;
    }
}

int tlb_lookup(const TLB *tlb,
               uint32_t pid,
               uint32_t vpn,
               uint32_t *pfn_out)
{

    uint32_t i;
    for (i = 0; i < TLB_ENTRIES; i++)
    {
        if (tlb->entry[i].valid == 0U)
        {
            continue;
        }
        if(tlb->entry[i].pid != pid)
        {
            continue;
        }
        if(tlb->entry[i].vpn != vpn)
        {
            continue;
        }
        *pfn_out = tlb->entry[i].pfn;
        return 1;
    }
    return 0;

}

int tlb_insert(TLB *tlb,
               uint32_t pid,
               uint32_t vpn,
               uint32_t pfn)
{

     uint32_t i;

    for (i = 0U; i < TLB_ENTRIES; i++)
    {
        if (
            tlb->entry[i].valid &&
            tlb->entry[i].pid == pid &&
            tlb->entry[i].vpn == vpn
        )
        {
            tlb->entry[i].pfn = pfn;
            return 1;
        }
    }
    for (i = 0U; i < TLB_ENTRIES; i++)
    {
        if (tlb->entry[i].valid == 0U)
        {
            tlb->entry[i].pid = pid;
            tlb->entry[i].vpn = vpn;
            tlb->entry[i].pfn = pfn;
            tlb->entry[i].valid = 1U;

            return 1;
        }
    }
    return 0;
}

void tlb_invalidate_pid(TLB *tlb, uint32_t pid)
{
    
    uint32_t i;
    for(i = 0; i < TLB_ENTRIES; i++)
    {
        if (tlb->entry[i].valid && tlb->entry[i].pid == pid)
        {
            tlb->entry[i].valid = 0U;
        }
    }

}

void tlb_invalidate_page(TLB *tlb, uint32_t pid, uint32_t vpn)
{

    uint32_t i;

    for (i = 0U; i < TLB_ENTRIES; i++)
    {
        if (tlb->entry[i].valid && tlb->entry[i].pid == pid && tlb->entry[i].vpn == vpn)
        {
            tlb->entry[i].valid = 0U;
        }
    }

}

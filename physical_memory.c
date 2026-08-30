#include "physical_memory.h"
#include <stdio.h>

void physical_memory_init(PhysicalMemory *mem)
{
    uint32_t i;
    
    mem->clock = 0U;

    for (i = 0; i < FRAME_COUNT; ++i) {
        mem->frame[i].occupied = 0U;
        mem->frame[i].pid = 0U;
        mem->frame[i].vpn = 0U;
        mem->frame[i].last_used = 0U;
    }
}

int physical_memory_find_free(const PhysicalMemory *mem, uint32_t *pfn_out)
{
    uint32_t i;
    for (i = 0U; i < FRAME_COUNT; i++)
    {
        if(mem->frame[i].occupied == 0U)
        {
            *pfn_out = i;
            return 1;
        }
    }
    return 0;
}

void physical_memory_assign(PhysicalMemory *mem, uint32_t pfn, uint32_t pid, uint32_t vpn)
{
    mem->frame[pfn].pid = pid;
    mem->frame[pfn].vpn = vpn;
    mem->frame[pfn].occupied = 1U;
    physical_memory_touch(mem, pfn);
}

void physical_memory_touch(PhysicalMemory *mem, uint32_t pfn)
{
    mem->clock++;
    mem->frame[pfn].last_used = mem->clock;
}

int physical_memory_get_frame(PhysicalMemory *mem,
                              uint32_t pid,
                              uint32_t vpn,
                              uint32_t *pfn_out)
{
    /*
     * TODO STUDENT:
     *
     * 1. Search for a free frame.
     * 2. If none, choose a GLOBAL LRU victim.
     * 3. Respect per-process lower/upper page limits.
     * 4. Return victim ownership as needed so the caller can invalidate
     *    the old PTE/TLB mapping.
     *
     * Exact lower/upper limits are not in the supplied Q1 page.
     */
    (void)mem;
    (void)pid;
    (void)vpn;
    (void)pfn_out;
    return 0;
}
void physical_memory_dump(
    const PhysicalMemory *mem
)
{
    uint32_t pfn;
    uint32_t occupied_count = 0U;

    printf("\n========== PHYSICAL MEMORY ==========\n");
    printf("PFN       PID   VPN       Last Used\n");
    printf("--------------------------------------\n");

    for (pfn = 0U; pfn < FRAME_COUNT; pfn++)
    {
        if (mem->frame[pfn].occupied)
        {
            printf(
                "0x%06X  %-4u  0x%06X  %llu\n",
                pfn,
                mem->frame[pfn].pid,
                mem->frame[pfn].vpn,
                (unsigned long long)
                mem->frame[pfn].last_used
            );

            occupied_count++;
        }
    }

    printf(
        "\nOccupied frames: %u / %u\n",
        occupied_count,
        FRAME_COUNT
    );
}
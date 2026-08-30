#include <stdio.h>

#include "memory_system.h"
#include "bitops.h"


static Process *find_process(
    MemorySystem *system,
    uint32_t pid
)
{
    uint32_t i;


    for (i = 0U; i < system->process_count; i++)
    {
        if (
            system->processes[i].pid == pid
        )
        {
            return
                &system->processes[i];
        }
    }


    return NULL;
}
static int choose_global_lru(
    MemorySystem *system,
    Process *requester,
    uint32_t *victim_pfn
)
{
    uint32_t i;

    uint32_t selected = 0U;

    uint8_t found = 0U;

    uint64_t oldest = 0U;


    for (i = 0U; i < FRAME_COUNT; i++)
    {
        Process *owner;


        if (
            system->memory.frame[i].occupied == 0U
        )
        {
            continue;
        }


        owner =
            find_process(
                system,
                system->memory.frame[i].pid
            );


        if (owner == NULL)
            continue;


        /*
         * If requester is already at upper limit,
         * victim must come from requester itself.
         */

        if (
            requester->resident_pages
            >=
            requester->upper_limit
        )
        {
            if (owner != requester)
                continue;
        }


        /*
         * Do not push another process below
         * its lower limit.
         *
         * If owner == requester, replacement leaves
         * its resident count unchanged.
         */

        if (
            owner != requester
            &&
            owner->resident_pages
            <=
            owner->lower_limit
        )
        {
            continue;
        }


        if (!found)
        {
            selected = i;

            oldest =
                system->memory.frame[i].last_used;

            found = 1U;
        }
        else if (
            system->memory.frame[i].last_used
            <
            oldest
        )
        {
            selected = i;

            oldest =
                system->memory.frame[i].last_used;
        }
    }


    if (!found)
        return 0;


    *victim_pfn = selected;

    return 1;
}
static int handle_page_fault(
    MemorySystem *system,
    Process *process,
    uint32_t vpn,
    uint32_t *pfn_out
)
{
    uint32_t pfn;


    /*
     * If process has not reached its upper limit,
     * first try a genuinely free frame.
     */

    if (
        process->resident_pages
        <
        process->upper_limit
    )
    {
        if (
            physical_memory_find_free(
                &system->memory,
                &pfn
            )
        )
        {
            physical_memory_assign(
                &system->memory,
                pfn,
                process->pid,
                vpn
            );


            page_table_map(
                &process->page_table,
                vpn,
                pfn
            );


            process->resident_pages++;


            *pfn_out = pfn;


            return 1;
        }
    }


    /*
     * No free frame OR requester reached upper limit.
     *
     * Global LRU.
     */

    if (
        !choose_global_lru(
            system,
            process,
            &pfn
        )
    )
    {
        return 0;
    }


    {
        uint32_t old_pid;
        uint32_t old_vpn;

        Process *old_process;


        old_pid =
            system->memory.frame[pfn].pid;

        old_vpn =
            system->memory.frame[pfn].vpn;


        old_process =
            find_process(
                system,
                old_pid
            );


        if (old_process != NULL)
        {
            page_table_unmap(
                &old_process->page_table,
                old_vpn
            );


            tlb_invalidate_page(
                &system->tlb,
                old_pid,
                old_vpn
            );


            /*
             * If another process lost the frame,
             * decrement its residency.
             *
             * If requester itself is replacing one
             * of its own pages, its total stays same.
             */

            if (old_process != process)
            {
                old_process->resident_pages--;

                process->resident_pages++;
            }
        }


        physical_memory_assign(
            &system->memory,
            pfn,
            process->pid,
            vpn
        );


        page_table_map(
            &process->page_table,
            vpn,
            pfn
        );
    }


    *pfn_out = pfn;


    return 1;
}
int memory_prepage(
    MemorySystem *system,
    uint32_t pid,
    uint32_t vpn
)
{
    Process *process;

    uint32_t pfn;


    process =
        find_process(
            system,
            pid
        );


    if (process == NULL)
        return 0;


    if (
        !handle_page_fault(
            system,
            process,
            vpn,
            &pfn
        )
    )
    {
        return 0;
    }


    /*
     * IMPORTANT:
     *
     * do NOT populate L1/L2.
     * do NOT pre-populate TLB.
     *
     * Only page table + physical frame.
     */


    return 1;
}
void memory_system_init(
    MemorySystem *system,
    Process *processes,
    uint32_t process_count
)
{
    system->processes =
        processes;

    system->process_count =
        process_count;


    tlb_init(
        &system->tlb
    );


    l1_init(
        &system->l1
    );


    l2_init(
        &system->l2
    );


    physical_memory_init(
        &system->memory
    );

    system->stats.accesses = 0U;

    system->stats.reads = 0U;
    system->stats.writes = 0U;

    system->stats.tlb_hits = 0U;
    system->stats.tlb_misses = 0U;

    system->stats.page_table_hits = 0U;
    system->stats.page_faults = 0U;

    system->stats.l1_hits = 0U;
    system->stats.l1_misses = 0U;

    system->stats.l2_hits = 0U;
    system->stats.l2_misses = 0U;

    system->stats.l2_writebacks = 0U;
}
int memory_access(
    MemorySystem *system,
    uint32_t pid,
    uint32_t va,
    MemoryOperation operation
)
{
        system->stats.accesses++;

    if (operation == MEM_READ)
    {
        system->stats.reads++;
    }
    else
    {
        system->stats.writes++;
    }
    Process *process;

    uint32_t vpn;
    uint32_t offset;

    uint32_t pfn;
    uint32_t pa;

    uint32_t l1_way;
    uint32_t l2_way;

    uint8_t writeback_required;

    uint32_t writeback_address;


    process =
        find_process(
            system,
            pid
        );


    if (process == NULL)
    {
        printf("Invalid PID\n");

        return 0;
    }


    /* =========================
       VIRTUAL ADDRESS WIRES
       ========================= */

    vpn =
        vpn_from_va(va);

    offset =
        page_offset_from_va(va);


    printf(
        "\nPID %u VA 0x%08X\n",
        pid,
        va
    );


    printf(
        "VPN=0x%X OFFSET=0x%X\n",
        vpn,
        offset
    );


    /* =========================
       TLB
       ========================= */

    if (
        tlb_lookup(
            &system->tlb,
            pid,
            vpn,
            &pfn
        )
    )
    {
        system->stats.tlb_hits++;

        printf("TLB HIT\n");
    }
    else
    {
        system->stats.tlb_misses++;

        printf("TLB MISS\n");


        /* =========================
           PAGE TABLE
           ========================= */

        if (
            page_table_lookup(
                &process->page_table,
                vpn,
                &pfn
            )
        )
        {
            system->stats.page_table_hits++;
            printf("PAGE TABLE HIT\n");
        }
        else
        {
            system->stats.page_faults++;
            printf("PAGE FAULT\n");


            if (
                !handle_page_fault(
                    system,
                    process,
                    vpn,
                    &pfn
                )
            )
            {
                printf(
                    "Unable to allocate frame\n"
                );

                return 0;
            }
        }


        /*
         * Cache translation in TLB.
         *
         * If TLB happens to be full and the
         * replacement policy is undefined,
         * translation still remains correct.
         */

        tlb_insert(
            &system->tlb,
            pid,
            vpn,
            pfn
        );
    }


    /* =========================
       PHYSICAL ADDRESS
       ========================= */

    pa =
        pa_from_pfn_offset(
            pfn,
            offset
        );


    printf(
        "PFN=0x%X PA=0x%08X\n",
        pfn,
        pa
    );


    physical_memory_touch(
        &system->memory,
        pfn
    );


    /* =========================
       L1
       ========================= */

    if (
        l1_lookup(
            &system->l1,
            va,
            pa,
            &l1_way
        )
    )
    {
        system->stats.l1_hits++;

        printf(
            "L1 HIT way=%u\n",
            l1_way
        );


        if (operation == MEM_READ)
        {
            return 1;
        }


        /*
         * WRITE THROUGH:
         *
         * even though L1 hit,
         * write continues downward.
         */
    }
    else
    {
        system->stats.l1_misses++;

        printf("L1 MISS\n");
    }


    /* =========================
       L2
       ========================= */

    if (
        l2_lookup(
            &system->l2,
            pa,
            &l2_way
        )
    )
    {
        system->stats.l2_hits++;

        printf(
            "L2 HIT way=%u\n",
            l2_way
        );
    }
    else
    {

        system->stats.l2_misses++;

        printf("L2 MISS -> MAIN MEMORY\n");


        l2_fill(
            &system->l2,
            pa,
            &writeback_required,
            &writeback_address,
            &l2_way
        );


        if (writeback_required)
        {
            system->stats.l2_writebacks++;
            printf(
                "L2 DIRTY WRITEBACK at PA 0x%08X\n",
                writeback_address
            );
        }
    }


    /* =========================
       READ
       ========================= */

    if (operation == MEM_READ)
    {
        /*
         * Requested block now exists in L2.
         *
         * Fill L1.
         */

        l1_fill(
            &system->l1,
            va,
            pa,
            &l1_way
        );


        printf(
            "FILLED L1 way=%u\n",
            l1_way
        );


        return 1;
    }


    /* =========================
       WRITE
       ========================= */

    /*
     * L1 is write-through:
     * therefore lower level receives the write.
     *
     * L2 is write-back:
     * therefore cached L2 copy becomes dirty.
     */

    l2_mark_dirty(
        &system->l2,
        pa,
        l2_way
    );


    /*
     * Whether L1 should allocate on a WRITE MISS
     * is not explicitly stated in the assignment.
     *
     * If your course assumes write-allocate:
     */

    l1_fill(
        &system->l1,
        va,
        pa,
        &l1_way
    );


    printf(
        "WRITE completed; L2 marked DIRTY\n"
    );


    return 1;
}
void memory_system_print_stats(
    const MemorySystem *system
)
{
    printf("\n");
    printf("========================================\n");
    printf("       MEMORY SYSTEM STATISTICS\n");
    printf("========================================\n");

    printf(
        "Total accesses       : %llu\n",
        (unsigned long long)system->stats.accesses
    );

    printf(
        "Reads                : %llu\n",
        (unsigned long long)system->stats.reads
    );

    printf(
        "Writes               : %llu\n",
        (unsigned long long)system->stats.writes
    );


    printf("\n--- TLB ---\n");

    printf(
        "TLB hits             : %llu\n",
        (unsigned long long)system->stats.tlb_hits
    );

    printf(
        "TLB misses           : %llu\n",
        (unsigned long long)system->stats.tlb_misses
    );


    printf("\n--- Paging ---\n");

    printf(
        "Page table hits      : %llu\n",
        (unsigned long long)system->stats.page_table_hits
    );

    printf(
        "Page faults          : %llu\n",
        (unsigned long long)system->stats.page_faults
    );


    printf("\n--- L1 ---\n");

    printf(
        "L1 hits              : %llu\n",
        (unsigned long long)system->stats.l1_hits
    );

    printf(
        "L1 misses            : %llu\n",
        (unsigned long long)system->stats.l1_misses
    );


    printf("\n--- L2 ---\n");

    printf(
        "L2 hits              : %llu\n",
        (unsigned long long)system->stats.l2_hits
    );

    printf(
        "L2 misses            : %llu\n",
        (unsigned long long)system->stats.l2_misses
    );

    printf(
        "L2 dirty writebacks  : %llu\n",
        (unsigned long long)system->stats.l2_writebacks
    );

    printf("========================================\n");
}
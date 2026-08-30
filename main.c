// #include <stdio.h>
// #include "config.h"
// #include "bitops.h"
// #include "tlb.h"
// #include "l1.h"
// #include "l2.h"
// #include "physical_memory.h"

// // /*
// //  * This driver is intentionally NOT a completed assignment solution.
// //  * It exposes the real flow so you can fill each hardware unit yourself.
// //  */

// // int main(void)
// // {
// //     TLB tlb;
// //     L1Cache l1;
// //     L2Cache l2;
// //     PhysicalMemory memory;

// //     uint32_t pid = 1U;
// //     uint32_t va  = 0x0000012CU;

// //     uint32_t vpn;
// //     uint32_t page_offset;
// //     uint32_t pfn = 0U;
// //     uint32_t pa;

// //     tlb_init(&tlb);
// //     l1_init(&l1);
// //     l2_init(&l2);
// //     physical_memory_init(&memory);

// //     /* CPU -> address wires */
// //     vpn = vpn_from_va(va);
// //     page_offset = page_offset_from_va(va);

// //     printf("CPU request\n");
// //     printf("  PID           = %u\n", pid);
// //     printf("  VA            = 0x%08X\n", va);
// //     printf("  VPN           = 0x%X\n", vpn);
// //     printf("  page offset   = 0x%X\n", page_offset);

// //     /*
// //      * Next stage should be:
// //      *
// //      * if (tlb_lookup(...)) {
// //      *     ...
// //      * } else {
// //      *     inspect process page table
// //      *     if absent -> page fault / global-LRU frame selection
// //      *     update PTE
// //      *     insert TLB entry
// //      * }
// //      *
// //      * This scaffold stops here because that is the assignment logic
// //      * you need to write and understand yourself.
// //      */

// //     (void)pfn;
// //     (void)pa;

// //     return 0;
// // }
// #include <stdio.h>

// #include "memory_system.h"


// int main(void)
// {
//     MemorySystem system;

//     Process processes[1];


//     /*
//      * Temporary example:
//      *
//      * PID        = 1
//      * pages      = 16
//      * lower      = 2
//      * upper      = 6
//      *
//      * Replace these limits with actual assignment input.
//      */

//     if (
//         !process_init(
//             &processes[0],
//             1U,
//             16U,
//             2U,
//             6U
//         )
//     )
//     {
//         printf(
//             "Process creation failed\n"
//         );

//         return 1;
//     }


//     memory_system_init(
//         &system,
//         processes,
//         1U
//     );


//     /*
//      * Common assumption:
//      * first two pages are pre-paged.
//      */

//     memory_prepage(
//         &system,
//         1U,
//         0U
//     );

//     memory_prepage(
//         &system,
//         1U,
//         1U
//     );


//     /*
//      * Later our actual test trace goes here.
//      */


//     process_destroy(
//         &processes[0]
//     );


//     return 0;
// }
#include <stdio.h>

#include "memory_system.h"


static void execute_read(
    MemorySystem *system,
    uint32_t pid,
    uint32_t va
)
{
    printf("\n");
    printf("========================================\n");
    printf("READ  PID=%u  VA=0x%08X\n", pid, va);
    printf("========================================\n");

    memory_access(
        system,
        pid,
        va,
        MEM_READ
    );
}


static void execute_write(
    MemorySystem *system,
    uint32_t pid,
    uint32_t va
)
{
    printf("\n");
    printf("========================================\n");
    printf("WRITE PID=%u  VA=0x%08X\n", pid, va);
    printf("========================================\n");

    memory_access(
        system,
        pid,
        va,
        MEM_WRITE
    );
}



int main(void)
{
    MemorySystem system;

    Process processes[1];


    /*
     * Temporary test process.
     *
     * 16 virtual pages.
     *
     * Lower/upper limits are TEST VALUES only.
     * Replace with whatever the assignment input
     * actually supplies.
     */

    if (
        !process_init(
            &processes[0],
            1U,
            16U,
            2U,
            12U
        )
    )
    {
        printf("Process initialization failed\n");

        return 1;
    }


    memory_system_init(
        &system,
        processes,
        1U
    );


    /*
     * Common assumption:
     *
     * Page 0 and page 1 are pre-paged.
     *
     * They enter MAIN MEMORY only.
     * Not TLB.
     * Not L1.
     * Not L2.
     */

    printf("PRE-PAGING VPN 0\n");

    memory_prepage(
        &system,
        1U,
        0U
    );


    printf("PRE-PAGING VPN 1\n");

    memory_prepage(
        &system,
        1U,
        1U
    );


    /*
     * ==========================================
     * TRACE 1
     *
     * Page 0 is already resident.
     *
     * Expected:
     *
     * TLB MISS
     * PAGE TABLE HIT
     * L1 MISS
     * L2 MISS
     * MAIN MEMORY
     * fill L2
     * fill L1
     * ==========================================
     */

    execute_read(
        &system,
        1U,
        0x0000012CU
    );


    /*
     * ==========================================
     * TRACE 2
     *
     * Exact same address.
     *
     * Expected:
     *
     * TLB HIT
     * L1 HIT
     * ==========================================
     */

    execute_read(
        &system,
        1U,
        0x0000012CU
    );


    /*
     * ==========================================
     * TRACE 3
     *
     * Page 1.
     *
     * VPN 1 is also already resident
     * because it was pre-paged.
     *
     * Expected:
     *
     * TLB MISS
     * PAGE TABLE HIT
     * cache activity
     * ==========================================
     */

    execute_read(
        &system,
        1U,
        0x0000052CU
    );


    /*
     * ==========================================
     * TRACE 4
     *
     * Page 2 has NOT been loaded.
     *
     * Expected:
     *
     * TLB MISS
     * PAGE FAULT
     * free physical frame allocated
     * PTE installed
     * TLB installed
     * cache access
     * ==========================================
     */

    execute_read(
        &system,
        1U,
        0x0000092CU
    );


    /*
     * ==========================================
     * TRACE 5
     *
     * Repeat page 2.
     *
     * Should now hit translation/cache paths.
     * ==========================================
     */

    execute_read(
        &system,
        1U,
        0x0000092CU
    );


    /*
     * ==========================================
     * TRACE 6
     *
     * Write to page 0.
     *
     * L1 is write-through.
     * L2 is write-back.
     *
     * Therefore this eventually marks the
     * corresponding L2 line DIRTY.
     * ==========================================
     */

    execute_write(
        &system,
        1U,
        0x0000012CU
    );


    /*
     * Print final statistics.
     */

    memory_system_print_stats(
        &system
    );


    process_destroy(
        &processes[0]
    );


    return 0;
}
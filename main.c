#include <stdio.h>
#include <stdint.h>

#include "memory_system.h"


/* ---------------------------------------------------------
   Small terminal-friendly wrapper around memory_access().
   --------------------------------------------------------- */

static void run_access(
    MemorySystem *system,
    uint32_t step,
    MemoryOperation operation,
    uint32_t pid,
    uint32_t va,
    const char *description
)
{
    const char *op_name;

    if (operation == MEM_READ)
        op_name = "READ";
    else
        op_name = "WRITE";


    printf("\n");
    printf("============================================================\n");
    printf(
        "[%02u] %-5s  PID=%u  VA=0x%08X\n",
        step,
        op_name,
        pid,
        va
    );

    printf("     %s\n", description);
    printf("------------------------------------------------------------\n");


    if (
        !memory_access(
            system,
            pid,
            va,
            operation
        )
    )
    {
        printf("!! ACCESS FAILED !!\n");
    }
}



/* =========================================================
   MAIN
   ========================================================= */

int main(void)
{
    MemorySystem system;

    Process processes[1];


    printf("\n");
    printf("============================================================\n");
    printf("              MEMORY HIERARCHY SIMULATOR\n");
    printf("============================================================\n");

    printf("Configuration:\n");
    printf("  Page size : 1 KB\n");
    printf("  TLB       : 32 entries, PID tagged\n");
    printf("  L1        : 4 KB, 16 B, 4-way\n");
    printf("  L2        : 32 KB, 32 B, 8-way\n");
    printf("  Memory    : 32 MB\n");
    printf("============================================================\n");


    /* -----------------------------------------------------
       Create one test process.

       16 virtual pages.

       lower_limit = 2
       upper_limit = 12

       These limits are TEMPORARY TEST VALUES.
       Replace them with the actual assignment/workload
       values when those are available.
       ----------------------------------------------------- */

    if (
        !process_init(
            &processes[0],
            1U,     /* PID */
            16U,    /* virtual pages */
            2U,     /* lower resident-page limit */
            12U     /* upper resident-page limit */
        )
    )
    {
        printf("\nERROR: Process initialization failed.\n");

        return 1;
    }


    /* -----------------------------------------------------
       Initialize complete hierarchy.
       ----------------------------------------------------- */

    memory_system_init(
        &system,
        processes,
        1U
    );


    /* -----------------------------------------------------
       PRE-PAGING

       Common assignment assumption:
       first two pages are loaded into MAIN MEMORY before
       execution begins.

       They are NOT inserted into:
           - TLB
           - L1
           - L2
       ----------------------------------------------------- */

    printf("\n");
    printf("[BOOT] Pre-paging PID 1: VPN 0 and VPN 1 ... ");


    if (
        !memory_prepage(
            &system,
            1U,
            0U
        )
    )
    {
        printf("FAILED\n");

        process_destroy(
            &processes[0]
        );

        return 1;
    }


    if (
        !memory_prepage(
            &system,
            1U,
            1U
        )
    )
    {
        printf("FAILED\n");

        process_destroy(
            &processes[0]
        );

        return 1;
    }


    printf("DONE\n");


    /* =====================================================
       TEST TRACE
       ===================================================== */


    /*
     * 1.
     *
     * VPN 0 is already in physical memory due to pre-paging.
     *
     * Expected:
     * TLB miss
     * Page-table hit
     * L1 miss
     * L2 miss
     * Main-memory access
     * Cache fill
     */

    run_access(
        &system,
        1U,
        MEM_READ,
        1U,
        0x0000012CU,
        "Cold read from pre-paged VPN 0"
    );


    /*
     * 2.
     *
     * Exact same address.
     *
     * Expected:
     * TLB hit
     * L1 hit
     */

    run_access(
        &system,
        2U,
        MEM_READ,
        1U,
        0x0000012CU,
        "Repeat previous read -- should exploit TLB and L1"
    );


    /*
     * 3.
     *
     * Same virtual page, different cache block.
     *
     * Translation should hit in TLB,
     * but this address has not yet been cached.
     *
     * Useful for showing:
     *
     * TLB hit != cache hit
     */

    run_access(
        &system,
        3U,
        MEM_READ,
        1U,
        0x0000016CU,
        "Same VPN, different cache block"
    );


    /*
     * 4.
     *
     * VPN 1 was also pre-paged.
     *
     * Expected:
     * TLB miss
     * Page-table hit
     * Cache activity
     */

    run_access(
        &system,
        4U,
        MEM_READ,
        1U,
        0x0000052CU,
        "First reference to second pre-paged page"
    );


    /*
     * 5.
     *
     * VPN 2 was NOT pre-paged.
     *
     * Expected:
     * TLB miss
     * Page fault
     * Free frame allocation
     * PTE installation
     * TLB installation
     * Cache fill
     */

    run_access(
        &system,
        5U,
        MEM_READ,
        1U,
        0x0000092CU,
        "Demand-paged access -- should produce a page fault"
    );


    /*
     * 6.
     *
     * Repeat VPN 2 access.
     *
     * Expected:
     * TLB hit
     * L1 hit
     */

    run_access(
        &system,
        6U,
        MEM_READ,
        1U,
        0x0000092CU,
        "Repeat demand-loaded address"
    );


    /*
     * 7.
     *
     * Write to an already cached address.
     *
     * Q1:
     *
     * L1 -> write-through
     * L2 -> write-back
     *
     * Therefore the write continues below L1 and
     * the corresponding L2 line becomes dirty.
     */

    run_access(
        &system,
        7U,
        MEM_WRITE,
        1U,
        0x0000012CU,
        "Write-through L1; corresponding L2 line becomes dirty"
    );


    /* =====================================================
       FINAL MACHINE STATE
       ===================================================== */

    printf("\n\n");
    printf("============================================================\n");
    printf("                    FINAL MACHINE STATE\n");
    printf("============================================================\n");


    memory_system_dump(
        &system
    );


    /* =====================================================
       FINAL STATISTICS
       ===================================================== */

    memory_system_print_stats(
        &system
    );


    /* -----------------------------------------------------
       Cleanup.
       ----------------------------------------------------- */

    process_destroy(
        &processes[0]
    );


    printf("\n");
    printf("Simulation completed successfully.\n\n");


    return 0;
}
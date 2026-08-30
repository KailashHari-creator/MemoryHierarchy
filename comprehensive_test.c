#include <stdio.h>
#include <stdint.h>

#include "memory_system.h"
#include "bitops.h"
#include "l1_lru.h"
#include "l2_lru.h"


/* =========================================================
   GLOBAL TEST RESULT COUNTERS
   ========================================================= */

static uint32_t pass_count = 0U;
static uint32_t fail_count = 0U;
static uint32_t warn_count = 0U;


/* =========================================================
   REPORT HELPERS
   ========================================================= */

static void section(const char *name)
{
    printf("\n\n");
    printf("============================================================\n");
    printf(" %s\n", name);
    printf("============================================================\n");
}


static void sub_section(const char *name)
{
    printf("\n");
    printf("------------------------------------------------------------\n");
    printf(" %s\n", name);
    printf("------------------------------------------------------------\n");
}


static void check_result(
    int condition,
    const char *description
)
{
    if (condition)
    {
        printf("[PASS] %s\n", description);

        pass_count++;
    }
    else
    {
        printf("[FAIL] %s\n", description);

        fail_count++;
    }
}


static void warning_result(
    int problem_detected,
    const char *description
)
{
    if (problem_detected)
    {
        printf("[WARN] %s\n", description);

        warn_count++;
    }
    else
    {
        printf("[PASS] %s\n", description);

        pass_count++;
    }
}


/* =========================================================
   ADDRESS CONSTRUCTION HELPERS

   Address construction remains shift / mask / OR based.
   ========================================================= */

static uint32_t make_va(
    uint32_t vpn,
    uint32_t offset
)
{
    return
        (vpn << PAGE_OFFSET_BITS)
        |
        (offset & 0x3FFU);
}


static uint32_t make_l2_pa(
    uint32_t tag,
    uint32_t set,
    uint32_t offset
)
{
    return
        (
            tag
            <<
            (
                L2_SET_BITS
                +
                L2_BLOCK_BITS
            )
        )
        |
        (
            set
            <<
            L2_BLOCK_BITS
        )
        |
        (
            offset
            &
            0x1FU
        );
}


/* =========================================================
   CACHE INSPECTION HELPERS
   ========================================================= */

static uint32_t l1_valid_count(
    const L1Cache *cache,
    uint32_t set
)
{
    uint32_t way;
    uint32_t count = 0U;

    for (way = 0U; way < L1_WAYS; way++)
    {
        if (cache->set[set].line[way].valid)
        {
            count++;
        }
    }

    return count;
}


static int l1_contains_tag(
    const L1Cache *cache,
    uint32_t set,
    uint32_t tag
)
{
    uint32_t way;

    for (way = 0U; way < L1_WAYS; way++)
    {
        if (
            cache->set[set].line[way].valid
            &&
            cache->set[set].line[way].virtual_tag == tag
        )
        {
            return 1;
        }
    }

    return 0;
}


static void print_l1_set(
    const L1Cache *cache,
    uint32_t set
)
{
    uint32_t way;
    uint32_t bit;

    printf("\nL1 SET %u\n", set);

    printf(
        "Predicted way: %u\n",
        cache->set[set].predicted_way
    );

    printf("\n");
    printf("Way   V   Virtual Tag\n");
    printf("---------------------\n");

    for (way = 0U; way < L1_WAYS; way++)
    {
        printf(
            "%3u   %u   0x%08X\n",
            way,
            cache->set[set].line[way].valid,
            cache->set[set].line[way].virtual_tag
        );
    }


    printf("\nLRU square matrix\n");

    for (way = 0U; way < L1_WAYS; way++)
    {
        printf("  ");

        for (bit = 0U; bit < L1_WAYS; bit++)
        {
            printf(
                "%u ",
                (
                    cache->set[set].lru.row[way]
                    >>
                    bit
                )
                &
                1U
            );
        }

        printf("\n");
    }
}


static uint32_t l2_valid_count(
    const L2Cache *cache,
    uint32_t set
)
{
    uint32_t way;
    uint32_t count = 0U;

    for (way = 0U; way < L2_WAYS; way++)
    {
        if (cache->set[set].line[way].valid)
        {
            count++;
        }
    }

    return count;
}


static int l2_contains_tag(
    const L2Cache *cache,
    uint32_t set,
    uint32_t tag
)
{
    uint32_t way;

    for (way = 0U; way < L2_WAYS; way++)
    {
        if (
            cache->set[set].line[way].valid
            &&
            cache->set[set].line[way].tag == tag
        )
        {
            return 1;
        }
    }

    return 0;
}


static void print_l2_set(
    const L2Cache *cache,
    uint32_t set
)
{
    uint32_t way;

    printf("\nL2 SET %u\n", set);

    printf("\n");
    printf("Way   V   D   Tag          LRU\n");
    printf("--------------------------------\n");

    for (way = 0U; way < L2_WAYS; way++)
    {
        printf(
            "%3u   %u   %u   0x%08X   %u\n",
            way,
            cache->set[set].line[way].valid,
            cache->set[set].line[way].dirty,
            cache->set[set].line[way].tag,
            cache->set[set].lru.counter[way]
        );
    }
}


/* =========================================================
   TEST 1
   BASIC END-TO-END MEMORY PATH
   ========================================================= */

static void test_basic_memory_path(void)
{
    MemorySystem system;

    Process process[1];

    uint32_t pfn;

    uint32_t va0;
    uint32_t va2;


    section("TEST 1 - BASIC END-TO-END MEMORY PATH");


    process_init(
        &process[0],
        1U,
        16U,
        2U,
        12U
    );


    memory_system_init(
        &system,
        process,
        1U
    );


    memory_prepage(
        &system,
        1U,
        0U
    );

    memory_prepage(
        &system,
        1U,
        1U
    );


    va0 =
        make_va(
            0U,
            0x12CU
        );


    va2 =
        make_va(
            2U,
            0x12CU
        );


    sub_section(
        "Cold access to pre-paged VPN 0"
    );


    memory_access(
        &system,
        1U,
        va0,
        MEM_READ
    );


    check_result(
        system.stats.tlb_misses == 1U,
        "Cold access produced a TLB miss"
    );


    check_result(
        system.stats.page_table_hits == 1U,
        "Pre-paged page produced page-table hit"
    );


    check_result(
        system.stats.l1_misses == 1U,
        "Cold access missed L1"
    );


    check_result(
        system.stats.l2_misses == 1U,
        "Cold access missed L2 and reached memory"
    );


    sub_section(
        "Repeat exact address"
    );


    memory_access(
        &system,
        1U,
        va0,
        MEM_READ
    );


    check_result(
        system.stats.tlb_hits == 1U,
        "Repeated access hit TLB"
    );


    check_result(
        system.stats.l1_hits == 1U,
        "Repeated access hit L1"
    );


    sub_section(
        "Demand-loaded VPN 2"
    );


    memory_access(
        &system,
        1U,
        va2,
        MEM_READ
    );


    check_result(
        system.stats.page_faults == 1U,
        "Previously absent VPN generated a page fault"
    );


    check_result(
        page_table_lookup(
            &process[0].page_table,
            2U,
            &pfn
        ),
        "Page fault installed VPN 2 into page table"
    );


    printf("\n");
    printf("Compact system snapshot after basic test:\n");

    memory_system_dump(
        &system
    );


    process_destroy(
        &process[0]
    );
}


/* =========================================================
   TEST 2
   L1 WAY PREDICTION + 4-WAY LRU EVICTION
   ========================================================= */

static void test_l1_prediction_and_eviction(void)
{
    MemorySystem system;

    Process process[1];

    uint32_t vpn;
    uint32_t va;

    uint32_t pfn;
    uint32_t pa;

    uint32_t set;

    uint32_t predicted_before;
    uint32_t hit_way;

    uint32_t victim_way;
    uint32_t victim_tag;

    uint32_t new_tag;


    section(
        "TEST 2 - L1 WAY PREDICTION AND SQUARE-MATRIX LRU"
    );


    process_init(
        &process[0],
        1U,
        16U,
        2U,
        10U
    );


    memory_system_init(
        &system,
        process,
        1U
    );


    memory_prepage(
        &system,
        1U,
        0U
    );

    memory_prepage(
        &system,
        1U,
        1U
    );


    /*
     * VPN 0..3 all use the same 0x12C page offset.
     *
     * L1 index lies entirely inside page offset,
     * therefore these references collide in one L1 set.
     */

    for (vpn = 0U; vpn < 4U; vpn++)
    {
        va =
            make_va(
                vpn,
                0x12CU
            );

        memory_access(
            &system,
            1U,
            va,
            MEM_READ
        );
    }


    set =
        l1_set_from_pa(
            0x12CU
        );


    check_result(
        l1_valid_count(
            &system.l1,
            set
        )
        ==
        4U,
        "Four colliding blocks filled all four L1 ways"
    );


    predicted_before =
        system.l1.set[set].predicted_way;


    /*
     * Access VPN 1.
     *
     * Predictor currently points at most recently filled
     * VPN 3, therefore prediction should be wrong but the
     * line should still be found in another way.
     */

    va =
        make_va(
            1U,
            0x12CU
        );


    page_table_lookup(
        &process[0].page_table,
        1U,
        &pfn
    );


    pa =
        pa_from_pfn_offset(
            pfn,
            0x12CU
        );


    check_result(
        l1_lookup(
            &system.l1,
            va,
            pa,
            &hit_way
        ),
        "L1 found block after wrong predicted-way probe"
    );


    check_result(
        predicted_before != hit_way,
        "The access genuinely exercised a wrong way prediction"
    );


    check_result(
        system.l1.set[set].predicted_way == hit_way,
        "Way predictor learned the actual hit way"
    );


    /*
     * Determine what the LRU hardware currently considers
     * the victim BEFORE performing the fifth fill.
     */

    victim_way =
        l1_lru_victim(
            &system.l1.set[set].lru
        );


    victim_tag =
        system.l1.set[set]
              .line[victim_way]
              .virtual_tag;


    check_result(
        victim_tag == 0U,
        "Square-matrix LRU identifies VPN/tag 0 as oldest"
    );


    /*
     * Fifth colliding line -> eviction.
     */

    va =
        make_va(
            4U,
            0x12CU
        );


    new_tag =
        l1_virtual_tag_from_va(
            va
        );


    memory_access(
        &system,
        1U,
        va,
        MEM_READ
    );


    check_result(
        l1_valid_count(
            &system.l1,
            set
        )
        ==
        4U,
        "L1 remains exactly four-way after replacement"
    );


    check_result(
        !l1_contains_tag(
            &system.l1,
            set,
            victim_tag
        ),
        "LRU victim was removed"
    );


    check_result(
        l1_contains_tag(
            &system.l1,
            set,
            new_tag
        ),
        "New fifth block replaced the victim"
    );


    print_l1_set(
        &system.l1,
        set
    );


    process_destroy(
        &process[0]
    );
}


/* =========================================================
   TEST 3
   L2 HIT + 8-WAY COUNTER LRU + DIRTY WRITEBACK
   ========================================================= */

static void test_l2_eviction_and_writeback(void)
{
    L2Cache cache;

    uint32_t set = 37U;

    uint32_t tag;
    uint32_t pa;

    uint32_t way;

    uint32_t victim;
    uint32_t old_tag;

    uint8_t writeback_required;

    uint32_t writeback_address;

    uint32_t expected_writeback;


    section(
        "TEST 3 - L2 8-WAY LRU AND DIRTY WRITEBACK"
    );


    l2_init(
        &cache
    );


    /*
     * Fill one specific L2 set with eight different tags.
     */

    for (tag = 1U; tag <= 8U; tag++)
    {
        pa =
            make_l2_pa(
                tag,
                set,
                0U
            );


        l2_fill(
            &cache,
            pa,
            &writeback_required,
            &writeback_address,
            &way
        );


        check_result(
            writeback_required == 0U,
            "Initial L2 fill required no dirty writeback"
        );
    }


    check_result(
        l2_valid_count(
            &cache,
            set
        )
        ==
        8U,
        "All eight L2 ways are occupied"
    );


    /*
     * Explicit L2 hit.
     */

    pa =
        make_l2_pa(
            5U,
            set,
            0U
        );


    check_result(
        l2_lookup(
            &cache,
            pa,
            &way
        ),
        "Existing L2 tag produced an L2 hit"
    );


    /*
     * With tags 1..8 filled sequentially and tag 5 touched,
     * tag 1 should remain oldest.
     */

    victim =
        l2_lru_victim(
            &cache.set[set].lru
        );


    old_tag =
        cache.set[set]
             .line[victim]
             .tag;


    check_result(
        old_tag == 1U,
        "LRU counter identifies tag 1 as eviction victim"
    );


    /*
     * Mark precisely that victim dirty.
     */

    pa =
        make_l2_pa(
            old_tag,
            set,
            0U
        );


    l2_mark_dirty(
        &cache,
        pa,
        victim
    );


    /*
     * Ninth line -> must evict one of eight ways.
     */

    pa =
        make_l2_pa(
            9U,
            set,
            0U
        );


    writeback_required = 0U;
    writeback_address = 0U;


    l2_fill(
        &cache,
        pa,
        &writeback_required,
        &writeback_address,
        &way
    );


    expected_writeback =
        make_l2_pa(
            old_tag,
            set,
            0U
        );


    check_result(
        writeback_required == 1U,
        "Evicting dirty L2 victim requested writeback"
    );


    check_result(
        writeback_address == expected_writeback,
        "Dirty writeback reconstructed correct block address"
    );


    check_result(
        !l2_contains_tag(
            &cache,
            set,
            old_tag
        ),
        "Old L2 victim tag is gone"
    );


    check_result(
        l2_contains_tag(
            &cache,
            set,
            9U
        ),
        "New L2 tag is resident"
    );


    check_result(
        l2_valid_count(
            &cache,
            set
        )
        ==
        8U,
        "L2 remains exactly eight-way after eviction"
    );


    print_l2_set(
        &cache,
        set
    );
}


/* =========================================================
   TEST 4
   GLOBAL PAGE LRU + UPPER LIMIT + TLB INVALIDATION

   Also checks current cache-invalidation behaviour.
   ========================================================= */

static void test_page_replacement(void)
{
    MemorySystem system;

    Process process[1];

    uint32_t va0;
    uint32_t va1;
    uint32_t va2;
    uint32_t va3;

    uint32_t pfn0;
    uint32_t tmp;

    uint32_t old_pa;

    uint32_t stale_way;

    uint64_t l2_hits_before;


    section(
        "TEST 4 - GLOBAL PAGE LRU, UPPER LIMIT AND INVALIDATION"
    );


    /*
     * Upper limit deliberately only 3 pages.
     */

    process_init(
        &process[0],
        1U,
        16U,
        2U,
        3U
    );


    memory_system_init(
        &system,
        process,
        1U
    );


    memory_prepage(
        &system,
        1U,
        0U
    );

    memory_prepage(
        &system,
        1U,
        1U
    );


    va0 = make_va(0U, 0x12CU);
    va1 = make_va(1U, 0x12CU);
    va2 = make_va(2U, 0x12CU);
    va3 = make_va(3U, 0x12CU);


    /*
     * Access 0, then 1, then demand-load 2.
     *
     * Resulting recency makes VPN 0 the oldest.
     */

    memory_access(
        &system,
        1U,
        va0,
        MEM_READ
    );

    memory_access(
        &system,
        1U,
        va1,
        MEM_READ
    );

    memory_access(
        &system,
        1U,
        va2,
        MEM_READ
    );


    check_result(
        process[0].resident_pages == 3U,
        "Process reached but did not exceed upper limit"
    );


    page_table_lookup(
        &process[0].page_table,
        0U,
        &pfn0
    );


    old_pa =
        pa_from_pfn_offset(
            pfn0,
            0x12CU
        );


    check_result(
        tlb_lookup(
            &system.tlb,
            1U,
            0U,
            &tmp
        ),
        "VPN 0 has TLB entry before replacement"
    );


    l2_hits_before =
        system.stats.l2_hits;


    /*
     * Process is already at upper limit.
     * VPN 3 must replace an existing resident page.
     */

    memory_access(
        &system,
        1U,
        va3,
        MEM_READ
    );


    check_result(
        process[0].resident_pages == 3U,
        "Resident-page count stayed at upper limit during replacement"
    );


    check_result(
        !page_table_lookup(
            &process[0].page_table,
            0U,
            &tmp
        ),
        "Global LRU evicted oldest VPN 0 PTE"
    );


    check_result(
        !tlb_lookup(
            &system.tlb,
            1U,
            0U,
            &tmp
        ),
        "TLB entry for evicted VPN 0 was invalidated"
    );


    check_result(
        system.memory.frame[pfn0].pid == 1U
        &&
        system.memory.frame[pfn0].vpn == 3U,
        "Victim physical frame was reassigned to VPN 3"
    );


    /*
     * CURRENT DESIGN CHECK:
     *
     * Correct page replacement should prevent a stale cache
     * line for the old physical page from satisfying the new
     * page's first access.
     */

    warning_result(
        system.stats.l2_hits > l2_hits_before,
        "Fresh page reused a stale L2 line after frame replacement "
        "(cache invalidation still needs review)"
    );


    warning_result(
        l1_lookup(
            &system.l1,
            va0,
            old_pa,
            &stale_way
        ),
        "Old VPN still has a matching L1 line after page eviction "
        "(cache invalidation still needs review)"
    );


    process_destroy(
        &process[0]
    );
}


/* =========================================================
   TEST 5
   TWO PIDS USING SAME VIRTUAL ADDRESS + TERMINATION
   ========================================================= */

static void test_pid_isolation(void)
{
    MemorySystem system;

    Process process[2];

    uint32_t va;

    uint32_t pfn1;
    uint32_t pfn2;

    uint32_t lookup_pfn;

    uint64_t l1_hits_before;


    section(
        "TEST 5 - PID ISOLATION AND PROCESS TERMINATION"
    );


    process_init(
        &process[0],
        1U,
        8U,
        2U,
        6U
    );


    process_init(
        &process[1],
        2U,
        8U,
        2U,
        6U
    );


    memory_system_init(
        &system,
        process,
        2U
    );


    /*
     * Common assumption: first two pages of each process
     * are pre-paged.
     */

    memory_prepage(&system, 1U, 0U);
    memory_prepage(&system, 1U, 1U);

    memory_prepage(&system, 2U, 0U);
    memory_prepage(&system, 2U, 1U);


    va =
        make_va(
            0U,
            0x12CU
        );


    page_table_lookup(
        &process[0].page_table,
        0U,
        &pfn1
    );


    page_table_lookup(
        &process[1].page_table,
        0U,
        &pfn2
    );


    check_result(
        pfn1 != pfn2,
        "Same VPN in two processes maps to different physical frames"
    );


    sub_section(
        "PID 1 accesses VA"
    );


    memory_access(
        &system,
        1U,
        va,
        MEM_READ
    );


    l1_hits_before =
        system.stats.l1_hits;


    sub_section(
        "PID 2 accesses exact same VA"
    );


    memory_access(
        &system,
        2U,
        va,
        MEM_READ
    );


    check_result(
        tlb_lookup(
            &system.tlb,
            1U,
            0U,
            &lookup_pfn
        )
        &&
        lookup_pfn == pfn1,
        "TLB preserves PID 1 translation"
    );


    check_result(
        tlb_lookup(
            &system.tlb,
            2U,
            0U,
            &lookup_pfn
        )
        &&
        lookup_pfn == pfn2,
        "TLB independently preserves PID 2 translation"
    );


    /*
     * Because our current L1 virtual tag does not include PID,
     * this test will expose whether two processes alias.
     */

    warning_result(
        system.stats.l1_hits > l1_hits_before,
        "PID 2 hit PID 1's virtually-tagged L1 entry "
        "(PID/context cache policy needs professor confirmation)"
    );


    sub_section(
        "PID-specific TLB invalidation on termination"
    );


    tlb_invalidate_pid(
        &system.tlb,
        1U
    );


    check_result(
        !tlb_lookup(
            &system.tlb,
            1U,
            0U,
            &lookup_pfn
        ),
        "Terminating PID 1 invalidates PID 1 TLB entries"
    );


    check_result(
        tlb_lookup(
            &system.tlb,
            2U,
            0U,
            &lookup_pfn
        ),
        "PID 2 TLB entries survive PID 1 termination"
    );


    process_destroy(
        &process[0]
    );

    process_destroy(
        &process[1]
    );
}


/* =========================================================
   TEST 6
   LOWER PAGE LIMIT PROTECTION

   This intentionally primes unused frames as occupied by a
   dummy PID so there is no free physical frame.

   That forces the GLOBAL replacement path without executing
   tens of thousands of memory references.
   ========================================================= */

static void test_lower_limit_protection(void)
{
    MemorySystem system;

    Process process[2];

    uint32_t i;

    uint32_t tmp;

    uint32_t va;


    section(
        "TEST 6 - LOWER RESIDENT-PAGE LIMIT PROTECTION"
    );


    /*
     * PID 1:
     * lower = 1
     *
     * PID 2:
     * lower = 2
     *
     * PID 2 will sit exactly at its lower limit.
     */

    process_init(
        &process[0],
        1U,
        8U,
        1U,
        3U
    );


    process_init(
        &process[1],
        2U,
        8U,
        2U,
        4U
    );


    memory_system_init(
        &system,
        process,
        2U
    );


    /*
     * Respect normal pre-paging requirement.
     */

    memory_prepage(&system, 1U, 0U);
    memory_prepage(&system, 1U, 1U);

    memory_prepage(&system, 2U, 0U);
    memory_prepage(&system, 2U, 1U);


    /*
     * Frames 0,1 -> PID 1
     * Frames 2,3 -> PID 2
     *
     * Artificially occupy every remaining frame with a
     * non-existent PID so physical_memory_find_free()
     * returns failure.
     *
     * choose_global_lru() ignores dummy PID because it has
     * no Process structure.
     */

    for (i = 4U; i < FRAME_COUNT; i++)
    {
        system.memory.frame[i].occupied = 1U;
        system.memory.frame[i].pid = 999U;
        system.memory.frame[i].vpn = 0U;
        system.memory.frame[i].last_used = 1000U;
    }


    /*
     * Make PID 2's pages globally oldest.
     *
     * They MUST still not be selected because PID 2 is
     * exactly at its lower limit.
     */

    system.memory.frame[2].last_used = 1U;
    system.memory.frame[3].last_used = 2U;

    system.memory.frame[0].last_used = 50U;
    system.memory.frame[1].last_used = 60U;


    va =
        make_va(
            2U,
            0x12CU
        );


    /*
     * PID 1 VPN 2 page fault.
     *
     * No free frames.
     *
     * Global LRU sees PID 2's older frames, but should
     * protect them due to PID 2's lower limit.
     */

    memory_access(
        &system,
        1U,
        va,
        MEM_READ
    );


    check_result(
        process[1].resident_pages == 2U,
        "PID 2 remained exactly at its lower page limit"
    );


    check_result(
        page_table_lookup(
            &process[1].page_table,
            0U,
            &tmp
        ),
        "PID 2 VPN 0 was protected"
    );


    check_result(
        page_table_lookup(
            &process[1].page_table,
            1U,
            &tmp
        ),
        "PID 2 VPN 1 was protected"
    );


    check_result(
        page_table_lookup(
            &process[0].page_table,
            2U,
            &tmp
        ),
        "Requester successfully loaded new page using eligible victim"
    );


    check_result(
        process[0].resident_pages == 2U,
        "Self-replacement left PID 1 resident-page count unchanged"
    );


    process_destroy(
        &process[0]
    );

    process_destroy(
        &process[1]
    );
}


/* =========================================================
   MAIN TEST RUNNER
   ========================================================= */

int main(void)
{
    FILE *report;


    /*
     * Redirect every printf from:
     *
     * - the test harness
     * - memory_access()
     * - state dumps
     *
     * into one readable report.
     */

    report =
        freopen(
            "memory_test_report.txt",
            "w",
            stdout
        );


    if (report == NULL)
    {
        fprintf(
            stderr,
            "Unable to create memory_test_report.txt\n"
        );

        return 1;
    }


    printf("============================================================\n");
    printf("          COMPREHENSIVE MEMORY SUBSYSTEM TEST REPORT\n");
    printf("============================================================\n");

    printf("\n");
    printf("Legend:\n");
    printf("  PASS : behaviour matched the expected model\n");
    printf("  FAIL : definite test failure\n");
    printf("  WARN : known/unresolved architectural behaviour detected\n");

    printf("\n");
    printf("This test intentionally stresses translation, paging,\n");
    printf("replacement, cache associativity, writeback and PID state.\n");


    test_basic_memory_path();

    test_l1_prediction_and_eviction();

    test_l2_eviction_and_writeback();

    test_page_replacement();

    test_pid_isolation();

    test_lower_limit_protection();


    section(
        "FINAL TEST SUMMARY"
    );


    printf(
        "PASS : %u\n",
        pass_count
    );

    printf(
        "FAIL : %u\n",
        fail_count
    );

    printf(
        "WARN : %u\n",
        warn_count
    );


    printf("\n");


    if (fail_count == 0U)
    {
        printf(
            "CORE TEST RESULT: PASS\n"
        );
    }
    else
    {
        printf(
            "CORE TEST RESULT: FAIL\n"
        );
    }


    if (warn_count != 0U)
    {
        printf("\n");
        printf(
            "Warnings indicate behaviours that should be discussed\n"
        );

        printf(
            "with the professor before final submission.\n"
        );
    }


    printf("\n");
    printf("============================================================\n");
    printf("                    END OF REPORT\n");
    printf("============================================================\n");


    fflush(stdout);


    fprintf(
        stderr,
        "\nTest complete.\n"
        "Open: memory_test_report.txt\n"
        "PASS=%u  FAIL=%u  WARN=%u\n\n",
        pass_count,
        fail_count,
        warn_count
    );


    /*
     * Hard failures make the executable return non-zero.
     * Warnings do not.
     */

    if (fail_count != 0U)
    {
        return 1;
    }


    return 0;
}
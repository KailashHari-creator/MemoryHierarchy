// #ifndef BITOPS_H
// #define BITOPS_H

// #include <stdint.h>
// #include "config.h"

// static inline uint32_t low_mask(uint32_t bits)
// {
//     return (1U << bits) - 1U;
// }

// static inline uint32_t vpn_from_va(uint32_t va)
// {
//     return va >> PAGE_OFFSET_BITS;
// }

// static inline uint32_t page_offset_from_va(uint32_t va)
// {
//     return va & low_mask(PAGE_OFFSET_BITS);
// }

// static inline uint32_t pa_from_pfn_offset(uint32_t pfn, uint32_t offset)
// {
//     return (pfn << PAGE_OFFSET_BITS) |
//            (offset & low_mask(PAGE_OFFSET_BITS));
// }

// /* L1: physically indexed, virtually tagged */
// static inline uint32_t l1_block_offset_from_pa(uint32_t pa)
// {
//     return pa & low_mask(L1_BLOCK_BITS);
// }

// static inline uint32_t l1_set_from_pa(uint32_t pa)
// {
//     return (pa >> L1_BLOCK_BITS) & low_mask(L1_SET_BITS);
// }

// static inline uint32_t l1_virtual_tag_from_va(uint32_t va)
// {
//     return va >> (L1_BLOCK_BITS + L1_SET_BITS);
// }

// /*
//  * L2 set/offset extraction from PA is provided because the cache must
//  * ultimately index some address. The assignment sheet does not explicitly
//  * state L2 tag/addressing semantics. Confirm the exact course convention
//  * before implementing l2_tag_from_*().
//  */
// static inline uint32_t l2_block_offset_from_pa(uint32_t pa)
// {
//     return pa & low_mask(L2_BLOCK_BITS);
// }

// static inline uint32_t l2_set_from_pa(uint32_t pa)
// {
//     return (pa >> L2_BLOCK_BITS) & low_mask(L2_SET_BITS);
// }

// static inline uint32_t l2_tag_from_pa(uint32_t pa)
// {
//     return pa >> (L2_BLOCK_BITS + L2_SET_BITS);
// }

// #endif
#ifndef BITOPS_H
#define BITOPS_H

#include <stdint.h>
#include "config.h"

static inline uint32_t low_mask(uint32_t bits)
{
    if (bits >= 32U)
    {
        return UINT32_MAX;
    }

    return (bits == 0U) ? 0U : ((1U << bits) - 1U);
}

/* =========================================================
   PAGING
   ========================================================= */

/*
 * Page size = 1 KB = 2^10
 *
 * VA:
 *
 * +----------------------+----------------+
 * |         VPN          | PAGE OFFSET    |
 * +----------------------+----------------+
 *                           bits [9:0]
 */

static inline uint32_t vpn_from_va(
    uint32_t va
)
{
    return va >> PAGE_OFFSET_BITS;
}


static inline uint32_t page_offset_from_va(
    uint32_t va
)
{
    return va & low_mask(PAGE_OFFSET_BITS);
}


static inline uint32_t pa_from_pfn_offset(
    uint32_t pfn,
    uint32_t offset
)
{
    return
        (pfn << PAGE_OFFSET_BITS)
        |
        (offset & low_mask(PAGE_OFFSET_BITS));
}



/* =========================================================
   L1 CACHE

   4 KB
   16 B blocks
   4 ways
   64 sets

   Offset = 4 bits
   Set    = 6 bits

   Q1:
   Physically Indexed
   Virtually Tagged
   ========================================================= */


/*
 * PA[3:0]
 */

static inline uint32_t l1_block_offset_from_pa(
    uint32_t pa
)
{
    return pa & low_mask(L1_BLOCK_BITS);
}


/*
 * PA[9:4]
 */

static inline uint32_t l1_set_from_pa(
    uint32_t pa
)
{
    return
        (pa >> L1_BLOCK_BITS)
        &
        low_mask(L1_SET_BITS);
}


/*
 * VA[31:10]
 *
 * Virtual tag because Q1 explicitly says
 * virtually tagged.
 */

static inline uint32_t l1_virtual_tag_from_va(
    uint32_t va
)
{
    return va >> (L1_BLOCK_BITS + L1_SET_BITS);
}

/*
 * L2 virtual tag for the VA-derived tag field used by the course Q1 convention.
 * This mirrors the L1 virtual-tag partitioning, but using the L2 geometry.
 */
static inline uint32_t l2_virtual_tag_from_va(
    uint32_t va
)
{
    return va >> (L2_BLOCK_BITS + L2_SET_BITS);
}



/* =========================================================
   L2 CACHE

   32 KB
   32 B blocks
   8 ways
   128 sets

   Offset = 5 bits
   Set    = 7 bits

   For now we are using PA for L2.
   ========================================================= */


/*
 * PA[4:0]
 */

static inline uint32_t l2_block_offset_from_pa(
    uint32_t pa
)
{
    return pa & low_mask(L2_BLOCK_BITS);
}


/*
 * PA[11:5]
 */

static inline uint32_t l2_set_from_pa(
    uint32_t pa
)
{
    return
        (pa >> L2_BLOCK_BITS)
        &
        low_mask(L2_SET_BITS);
}


/*
 * PA[31:12]
 */

static inline uint32_t l2_tag_from_pa(
    uint32_t pa
)
{
    return pa >> (L2_BLOCK_BITS + L2_SET_BITS);
}


#endif
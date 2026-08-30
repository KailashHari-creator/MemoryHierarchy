#include <assert.h>
#include <stdio.h>
#include "config.h"
#include "bitops.h"

static void test_one_translation_shape(void)
{
    /*
     * Hand-constructed test:
     *
     * VA  = 0x0000012C
     * VPN = 0
     * page offset = 0x12C
     *
     * Pretend the page table/TLB maps VPN 0 -> PFN 0x12.
     * Then:
     * PA = (0x12 << 10) | 0x12C = 0x492C.
     */

    uint32_t va = 0x0000012CU;
    uint32_t pfn = 0x12U;

    uint32_t vpn = vpn_from_va(va);
    uint32_t po  = page_offset_from_va(va);
    uint32_t pa  = pa_from_pfn_offset(pfn, po);

    assert(vpn == 0x0U);
    assert(po  == 0x12CU);
    assert(pa  == 0x492CU);

    /*
     * L1 geometry:
     * offset = PA[3:0]      = 0xC
     * set    = PA[9:4]      = 18
     * vtag   = VA[31:10]    = 0
     */
    assert(l1_block_offset_from_pa(pa) == 0xCU);
    assert(l1_set_from_pa(pa)          == 18U);
    assert(l1_virtual_tag_from_va(va)  == 0U);

    /*
     * L2 geometry only:
     * offset = PA[4:0]      = 0xC
     * set    = PA[11:5]     = 73
     */
    assert(l2_block_offset_from_pa(pa) == 0xCU);
    assert(l2_set_from_pa(pa)          == 73U);
}

static void test_page_boundary_bits(void)
{
    uint32_t va_a = 0x000003FFU;
    uint32_t va_b = 0x00000400U;

    assert(vpn_from_va(va_a) == 0U);
    assert(page_offset_from_va(va_a) == 0x3FFU);

    assert(vpn_from_va(va_b) == 1U);
    assert(page_offset_from_va(va_b) == 0U);
}

static void test_l1_set_wrap(void)
{
    /*
     * Changing PA bit 10 must NOT alter the 6-bit L1 set field PA[9:4].
     */
    uint32_t pa_a = 0x000003F0U;
    uint32_t pa_b = 0x000007F0U;

    assert(l1_set_from_pa(pa_a) == 63U);
    assert(l1_set_from_pa(pa_b) == 63U);
}

int main(void)
{
    test_one_translation_shape();
    test_page_boundary_bits();
    test_l1_set_wrap();

    puts("PASS: all bit-level geometry tests succeeded.");
    return 0;
}

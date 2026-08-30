#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

/* Question 1 configuration */
#define PAGE_OFFSET_BITS 10U
#define PAGE_SIZE         (1U << PAGE_OFFSET_BITS)

#define TLB_ENTRIES       32U

#define L1_SIZE_BITS      12U
#define L1_SIZE           (1U << L1_SIZE_BITS)
#define L1_BLOCK_BITS     4U
#define L1_BLOCK_SIZE     (1U << L1_BLOCK_BITS)
#define L1_WAYS           4U
#define L1_SET_BITS       6U
#define L1_SETS           (1U << L1_SET_BITS)

#define L2_SIZE_BITS      15U
#define L2_SIZE           (1U << L2_SIZE_BITS)
#define L2_BLOCK_BITS     5U
#define L2_BLOCK_SIZE     (1U << L2_BLOCK_BITS)
#define L2_WAYS           8U
#define L2_SET_BITS       7U
#define L2_SETS           (1U << L2_SET_BITS)

#define PHYS_MEM_BITS     25U
#define PHYS_MEM_SIZE     (1U << PHYS_MEM_BITS)
#define PFN_BITS          (PHYS_MEM_BITS - PAGE_OFFSET_BITS)
#define FRAME_COUNT       (1U << PFN_BITS)

#endif

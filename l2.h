#ifndef L2_H
#define L2_H

#include <stdint.h>
#include "config.h"
#include "l2_lru.h"
#include "bitops.h"

typedef struct {
    uint8_t  valid;
    uint8_t  dirty;

    /*
     * The assignment sheet does not explicitly state L2 virtual/physical
     * tag semantics. Add the tag field required by your lecture convention.
     */
    uint32_t tag;

    uint8_t data[L2_BLOCK_SIZE];
} L2Line;

typedef struct {
    L2Line line[L2_WAYS];
    L2LRUCounter lru;
} L2Set;

typedef struct {
    L2Set set[L2_SETS];
} L2Cache;

void l2_init(L2Cache *cache);

/*
 * TODO:
 * implement lookup only after fixing the exact L2 tag/address convention
 * from the course material.
 */
int l2_lookup(L2Cache *cache,
              uint32_t pa,
              uint32_t *way_out);

int l2_fill(
    L2Cache *cache,
    uint32_t pa,
    uint8_t *writeback_required,
    uint32_t *writeback_address,
    uint32_t *way_out
);


void l2_mark_dirty(
    L2Cache *cache,
    uint32_t pa,
    uint32_t way
);

#endif

#ifndef L1_H
#define L1_H

#include <stdint.h>
#include "config.h"
#include "l1_lru.h"
#include "bitops.h"

typedef struct {
    uint8_t  valid;
    uint32_t virtual_tag;
    uint8_t  data[L1_BLOCK_SIZE];
} L1Line;

typedef struct {
    L1Line line[L1_WAYS];
    uint8_t predicted_way;
    L1LRUMatrix lru;
} L1Set;

typedef struct {
    L1Set set[L1_SETS];
} L1Cache;

void l1_init(L1Cache *cache);

void l1_dump(const L1Cache *cache);

int l1_lookup(L1Cache *cache,
              uint32_t va,
              uint32_t pa,
              uint32_t *way_out);

int l1_fill(L1Cache *cache,
    uint32_t va,
    uint32_t pa,
    uint32_t *way_out
);

#endif

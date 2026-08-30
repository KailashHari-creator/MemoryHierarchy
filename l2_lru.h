#ifndef L2_LRU_H
#define L2_LRU_H

#include <stdint.h>
#include "config.h"

typedef struct {
    uint8_t counter[L2_WAYS];
} L2LRUCounter;

void l2_lru_init(L2LRUCounter *lru);
void l2_lru_touch(L2LRUCounter *lru, uint32_t way);
uint32_t l2_lru_victim(const L2LRUCounter *lru);

#endif

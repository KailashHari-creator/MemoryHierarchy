#ifndef L1_LRU_H
#define L1_LRU_H

#include <stdint.h>
#include "config.h"


typedef struct {
    uint8_t row[L1_WAYS];
} L1LRUMatrix;

void l1_lru_init(L1LRUMatrix *m);

void l1_lru_touch(L1LRUMatrix *m, uint32_t way);

uint32_t l1_lru_victim(const L1LRUMatrix *m);

#endif

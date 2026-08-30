#include "l2_lru.h"

void l2_lru_init(L2LRUCounter *lru)
{
    uint32_t i;
    for (i = 0; i < L2_WAYS; ++i)
        lru->counter[i] = 0U;
}

void l2_lru_touch(L2LRUCounter *lru, uint32_t way)
{

    uint32_t i;
    uint8_t old_rank;
    old_rank = lru->counter[way];
    for(i = 0; i < L2_WAYS; i++)
    {
        if (i == way)
        {
            continue;
        }
        if(lru->counter[i] < old_rank)
        {
            lru->counter[i]++;
        }
    }
    lru->counter[way] = 0U;

}

uint32_t l2_lru_victim(const L2LRUCounter *lru)
{
    uint32_t i;
    uint32_t victim = 0U;
    uint8_t largest = lru->counter[0];

    for(i = 1U; i < L2_WAYS; i++)
    {
        if (lru->counter[i] > largest)
        {
            largest = lru->counter[i];
            victim = i;
        }

    }

    return victim;
}

#include "l1_lru.h"

void l1_lru_init(L1LRUMatrix *m)
{
    uint32_t i;
    for (i = 0; i < L1_WAYS; ++i)
        m->row[i] = 0U;
}

void l1_lru_touch(L1LRUMatrix *m, uint32_t way)
{

    uint32_t i;
    uint32_t way_bit;

    for (i = 0U; i < L1_WAYS; i++)
    {
        if (i == way)
        {
            continue;
        }
        m->row[way] |= (uint8_t)(1U << i);
        way_bit = (uint8_t)(1U << way);
        m->row[i] &= (uint8_t)(~way_bit);
    }
    m->row[way] &= (uint8_t)(~(1U << way));

}

uint32_t l1_lru_victim(const L1LRUMatrix *m)
{
    uint32_t way;
    uint32_t mask = (uint8_t)((1 << L1_WAYS)-1U);
    for (way = 0U; way < L1_WAYS; way++)
    {
        if ((m->row[way] & mask) == 0U)
        {
            return way;
        }
    }
    return 0U;
}

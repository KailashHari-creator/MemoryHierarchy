#include "l1.h"

void l1_init(L1Cache *cache)
{
    uint32_t s;
    uint32_t w;

    for (s = 0; s < L1_SETS; ++s) {
        cache->set[s].predicted_way = 0U;
        l1_lru_init(&cache->set[s].lru);

        for (w = 0; w < L1_WAYS; ++w) {
            cache->set[s].line[w].valid = 0U;
            cache->set[s].line[w].virtual_tag = 0U;
        }
    }
}

int l1_lookup(L1Cache *cache,
              uint32_t va,
              uint32_t pa,
              uint32_t *way_out)
{
    
    uint32_t set;
    uint32_t tag;
    uint32_t predicted;
    uint32_t way;

    set = l1_set_from_pa(pa);
    tag = l1_virtual_tag_from_va(va);
    predicted = cache->set[set].predicted_way;

    if(cache->set[set].line[predicted].valid && cache->set[set].line[predicted].virtual_tag == tag)
    {
        *way_out = predicted;
        l1_lru_touch( &cache->set[set].lru, predicted );

        return 1;
    }
    
    for (way = 0U; way < L1_WAYS; way++)
    {
        if (way == predicted)
            continue;


        if ( cache->set[set].line[way].valid && cache->set[set].line[way].virtual_tag == tag )
        {
            *way_out = way;

            cache->set[set].predicted_way = (uint8_t)way;

            l1_lru_touch( &cache->set[set].lru, way );

            return 1;
        }
    }

    return 0;
}
int l1_fill(L1Cache *cache, uint32_t va, uint32_t pa, uint32_t *way_out)
{
    uint32_t set;
    uint32_t tag;
    uint32_t way;

    set = l1_set_from_pa(pa);

    tag = l1_virtual_tag_from_va(va);

    for (way = 0U; way < L1_WAYS; way++)
    {
        if ( cache->set[set].line[way].valid == 0U )
        {
            break;
        }
    }

    if (way == L1_WAYS)
    {
        way = l1_lru_victim( &cache->set[set].lru);
    }

    cache->set[set].line[way].virtual_tag = tag;

    cache->set[set].line[way].valid = 1U;

    l1_lru_touch( &cache->set[set].lru, way );

    cache->set[set].predicted_way = (uint8_t)way;

    *way_out = way;

    return 1;
}

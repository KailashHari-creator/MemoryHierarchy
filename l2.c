#include "l2.h"

#include <stdio.h>

void l2_init(L2Cache *cache)
{
    uint32_t s;
    uint32_t w;

    for (s = 0; s < L2_SETS; ++s) {
        l2_lru_init(&cache->set[s].lru);

        for (w = 0; w < L2_WAYS; ++w) {
            cache->set[s].line[w].valid = 0U;
            cache->set[s].line[w].dirty = 0U;
            cache->set[s].line[w].tag = 0U;
        }
    }
}

int l2_lookup(L2Cache *cache,
              uint32_t pa,
              uint32_t *way_out)
{

    uint32_t tag;
    uint32_t set;
    uint32_t way;

    set = l2_set_from_pa(pa);
    tag = l2_tag_from_pa(pa);

    for(way = 0; way < L2_WAYS; way++)
    {
        if (cache->set[set].line[way].valid && cache->set[set].line[way].tag == tag)
        {
            * way_out = way;
            l2_lru_touch(&cache->set[set].lru, way);
            return 1;
        }
    }

    return 0;
}
int l2_fill(
    L2Cache *cache,
    uint32_t pa,
    uint8_t *writeback_required,
    uint32_t *writeback_address,
    uint32_t *way_out
)
{
    uint32_t set;
    uint32_t tag;
    uint32_t way;


    set = l2_set_from_pa(pa);
    tag = l2_tag_from_pa(pa);
    *writeback_required = 0U;

    for (way = 0U; way < L2_WAYS; way++)
    {
        if (cache->set[set].line[way].valid == 0U)
        {
            break;
        }
    }

    if (way == L2_WAYS)
    {
        way = l2_lru_victim(&cache->set[set].lru);

        if (cache->set[set].line[way].dirty)
        {
            *writeback_required = 1U;
            *writeback_address = (cache->set[set].line[way].tag << (L2_SET_BITS+L2_BLOCK_BITS)) | (set<<L2_BLOCK_BITS);
        }
    }


    cache->set[set].line[way].tag = tag;
    cache->set[set].line[way].valid = 1U;
    cache->set[set].line[way].dirty = 0U;

    l2_lru_touch(&cache->set[set].lru, way);

    *way_out = way;
    return 1;
}
void l2_mark_dirty(
    L2Cache *cache,
    uint32_t pa,
    uint32_t way
)
{
    uint32_t set;

    set =
        l2_set_from_pa(pa);

    cache->set[set].line[way].dirty =
        1U;
}
void l2_dump(
    const L2Cache *cache
)
{
    uint32_t set;
    uint32_t way;
    uint8_t any_valid;

    printf("\n========== L2 CACHE ==========\n");

    for (set = 0U; set < L2_SETS; set++)
    {
        any_valid = 0U;

        for (way = 0U; way < L2_WAYS; way++)
        {
            if (cache->set[set].line[way].valid)
            {
                any_valid = 1U;
                break;
            }
        }

        if (!any_valid)
            continue;


        printf(
            "\nSET %u\n",
            set
        );

        printf(
            "Way   V   D   Tag          LRU\n"
        );

        printf(
            "--------------------------------\n"
        );


        for (way = 0U; way < L2_WAYS; way++)
        {
            printf(
                "%3u   %u   %u   0x%08X   %u\n",
                way,
                cache->set[set].line[way].valid,
                cache->set[set].line[way].dirty,
                cache->set[set].line[way].tag,
                cache->set[set].lru.counter[way]
            );
        }
    }
}
#ifndef PHYSICAL_MEMORY_H
#define PHYSICAL_MEMORY_H

#include <stdint.h>
#include "config.h"

typedef struct
{
    uint8_t occupied;

    uint32_t pid;
    uint32_t vpn;

    /* timestamp used for global LRU */
    uint64_t last_used;

} FrameMeta;


typedef struct
{
    FrameMeta frame[FRAME_COUNT];

    /* global logical clock for LRU */
    uint64_t clock;

} PhysicalMemory;


void physical_memory_init(
    PhysicalMemory *mem
);


int physical_memory_find_free(
    const PhysicalMemory *mem,
    uint32_t *pfn_out
);


void physical_memory_assign(
    PhysicalMemory *mem,
    uint32_t pfn,
    uint32_t pid,
    uint32_t vpn
);


void physical_memory_touch(
    PhysicalMemory *mem,
    uint32_t pfn
);


#endif
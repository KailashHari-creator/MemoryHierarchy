#ifndef MEMORY_SYSTEM_H
#define MEMORY_SYSTEM_H

#include <stdint.h>

#include "tlb.h"
#include "l1.h"
#include "l2.h"
#include "physical_memory.h"
#include "process.h"

typedef struct
{
    uint64_t accesses;

    uint64_t reads;
    uint64_t writes;

    uint64_t tlb_hits;
    uint64_t tlb_misses;

    uint64_t page_table_hits;
    uint64_t page_faults;

    uint64_t l1_hits;
    uint64_t l1_misses;

    uint64_t l2_hits;
    uint64_t l2_misses;

    uint64_t l2_writebacks;

} MemoryStats;


typedef struct
{
    TLB tlb;

    L1Cache l1;

    L2Cache l2;

    PhysicalMemory memory;

    Process *processes;

    uint32_t process_count;

    MemoryStats stats;

} MemorySystem;


typedef enum
{
    MEM_READ = 0,
    MEM_WRITE = 1

} MemoryOperation;


void memory_system_init(
    MemorySystem *system,
    Process *processes,
    uint32_t process_count
);


int memory_access(
    MemorySystem *system,
    uint32_t pid,
    uint32_t va,
    MemoryOperation operation
);


int memory_prepage(
    MemorySystem *system,
    uint32_t pid,
    uint32_t vpn
);


void memory_system_print_stats(
    const MemorySystem *system
);

void memory_system_dump(
    const MemorySystem *system
);

#endif
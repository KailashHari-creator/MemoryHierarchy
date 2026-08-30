#ifndef TLB_H
#define TLB_H

#include <stdint.h>
#include "config.h"

typedef struct {
    uint8_t  valid;
    uint32_t pid;
    uint32_t vpn;
    uint32_t pfn;
} TLBEntry;

typedef struct {
    TLBEntry entry[TLB_ENTRIES];
} TLB;

void tlb_init(TLB *tlb);

/*
 * Return 1 on hit and place FN in *pfn_out.
 * Return 0 on miss.
 *
 * TODO: implement by explicitly comparing valid + PID + VPN
 * against the 32 entries.
 */
int tlb_lookup(const TLB *tlb,
               uint32_t pid,
               uint32_t vpn,
               uint32_t *pfn_out);

/*
 * TODO:
 * - first handle an already-existing mapping
 * - then use an invalid entry if available
 * - the assignment sheet does NOT state a TLB replacement policy
 *   for the full-TLB case, so do not invent one without course guidance.
 */
int tlb_insert(TLB *tlb,
               uint32_t pid,
               uint32_t vpn,
               uint32_t pfn);

/* Invalidate all translations belonging to one PID. */
void tlb_invalidate_pid(TLB *tlb, uint32_t pid);

/* Useful when a resident page is evicted from physical memory. */
void tlb_invalidate_page(TLB *tlb, uint32_t pid, uint32_t vpn);

#endif

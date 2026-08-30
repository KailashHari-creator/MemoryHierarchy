#ifndef PAGE_TABLE_H
#define PAGE_TABLE_H

#include <stdint.h>

typedef struct {
    uint8_t  present;
    uint32_t pfn;
} PageTableEntry;

typedef struct {
    PageTableEntry *pte;
    uint32_t page_count;
} PageTable;

int page_table_init(PageTable *pt, uint32_t page_count);
void page_table_destroy(PageTable *pt);

int page_table_lookup(const PageTable *pt,
                      uint32_t vpn,
                      uint32_t *pfn_out);

int page_table_map(PageTable *pt,
                   uint32_t vpn,
                   uint32_t pfn);

int page_table_unmap(PageTable *pt,
                     uint32_t vpn);

void page_table_dump(
    const PageTable *pt,
    uint32_t pid
);

#endif

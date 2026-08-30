#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include "page_table.h"

typedef struct {
    uint32_t pid;
    PageTable page_table;
    uint32_t resident_pages;
    uint32_t lower_limit;
    uint32_t upper_limit;
} Process;

int process_init(Process *p, uint32_t pid, uint32_t page_count, uint32_t lower_limit, uint32_t upper_limit);
void process_destroy(Process *p);

#endif

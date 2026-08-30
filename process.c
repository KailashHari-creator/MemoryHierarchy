#include "process.h"

int process_init(Process *p, uint32_t pid, uint32_t page_count, uint32_t lower_limit, uint32_t upper_limit)
{
    p->pid = pid;
    p->resident_pages = 0U;
    p->lower_limit = lower_limit;
    p->upper_limit = upper_limit;
    return page_table_init(&p->page_table, page_count);
}

void process_destroy(Process *p)
{
    page_table_destroy(&p->page_table);
    p->resident_pages = 0U;
}

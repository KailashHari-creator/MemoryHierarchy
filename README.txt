DSTN Q1 LEARNING SCAFFOLD
=========================

This is intentionally not a completed assignment implementation.

Files:
- config.h              Q1 power-of-two hardware dimensions
- bitops.h              shift/mask-only address field extraction
- tlb.h/.c              32-entry PID-tagged TLB structure and TODO logic
- page_table.h/.c       pure-paging PTE structure and TODO logic
- process.h/.c          minimal process holder
- physical_memory.h/.c  2^15 frame metadata structure and TODO global-LRU logic
- l1.h/.c               64-set x 4-way L1 structure and TODO lookup logic
- l1_lru.h/.c           packed 4x4 square-matrix LRU scaffold
- l2.h/.c               128-set x 8-way L2 structure and TODO lookup logic
- l2_lru.h/.c           8-way LRU-counter scaffold
- main.c                explicit CPU -> translation flow starting point
- test_geometry.c       working tests for address bit wiring
- TEST_PLAN.md           deterministic full-flow tests to implement
- Makefile               builds the scaffold and geometry test

Compile:
    make

Run geometry tests:
    make test

Clean:
    make clean

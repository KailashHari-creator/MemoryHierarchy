# Q1 deterministic test plan

This is a development test plan, not a supplied assignment input format.

## Geometry test already implemented

The included `test_geometry.c` verifies only bit-level address wiring:

- 1 KB page split: VPN = VA >> 10, offset = VA[9:0]
- PA reconstruction: PFN concatenated with page offset
- L1: PA[3:0] block offset, PA[9:4] physical set index
- L1 virtual tag: VA bits above index+offset
- L2 geometry: PA[4:0] block offset and PA[11:5] set index

Run:

    make test

Expected:

    PASS: all bit-level geometry tests succeeded.

## Full simulator trace to build after your TODOs

Use one small process whose first two pages are pre-paged.

Choose deterministic page mappings for the test only:

- PID 1, VPN 0 -> PFN 0x12
- PID 1, VPN 1 -> PFN 0x34

Start with an empty TLB and empty caches.

### Access A
PID 1 reads VA 0x0000012C

Expected control flow:

1. VPN = 0
2. page offset = 0x12C
3. TLB miss
4. page-table hit because VPN 0 is pre-paged
5. PFN = 0x12
6. TLB entry inserted
7. PA = 0x492C
8. L1 physical set = 18
9. L1 virtual tag = 0
10. L1 initially misses
11. Continue according to your course's exact look-through/look-aside rules.

### Access B
Repeat PID 1 read VA 0x0000012C

Expected:

1. Same VPN and offset
2. TLB hit this time
3. Same PA
4. If Access A filled L1 according to your course rules, this should now exercise
   the L1-hit and way-prediction path.

### Access C
PID 1 reads VA 0x0000052C

Because 0x52C = VPN 1 with offset 0x12C:

1. TLB miss for VPN 1
2. page-table hit because page 1 is pre-paged
3. PFN = 0x34
4. PA = (0x34 << 10) | 0x12C
5. Exercise a second cache mapping.

### Access D
PID 1 reads an address in VPN 2

Expected:

1. TLB miss
2. PTE not present
3. page fault
4. obtain a free frame if one exists
5. update PTE
6. add TLB translation
7. continue cache access

Do NOT force a page-replacement test until you know:
- the lower resident-page limit,
- the upper resident-page limit,
because the assignment explicitly says both must be maintained but the supplied
question page does not give their numeric values.

## Important unresolved course-specific points

Do not invent these from this scaffold:

1. TLB replacement policy when all 32 entries are valid.
2. Exact lower/upper resident-page limits.
3. Exact implementation semantics of "look through" and "look aside" as taught
   in your course.
4. Exact L2 tag/address convention.
5. Exact square-matrix LRU bit convention.
6. Exact LRU-counter update convention.

Use your lecture material/professor's definition for all six.

CC = gcc

CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O0 -g


CORE_OBJS = \
	tlb.o \
	page_table.o \
	process.o \
	physical_memory.o \
	l1_lru.o \
	l1.o \
	l2_lru.o \
	l2.o \
	memory_system.o


all: simulator comprehensive_test


simulator: main.o $(CORE_OBJS)
	$(CC) $(CFLAGS) -o simulator main.o $(CORE_OBJS)


comprehensive_test: comprehensive_test.o $(CORE_OBJS)
	$(CC) $(CFLAGS) -o comprehensive_test comprehensive_test.o $(CORE_OBJS)


comprehensive_test.o: comprehensive_test.c
	$(CC) $(CFLAGS) -c comprehensive_test.c


test_geometry: $(TEST_OBJS)
	$(CC) $(CFLAGS) -o test_geometry $(TEST_OBJS)


test: test_geometry
	./test_geometry


main.o: main.c
	$(CC) $(CFLAGS) -c main.c


test_geometry.o: test_geometry.c config.h bitops.h
	$(CC) $(CFLAGS) -c test_geometry.c


tlb.o: tlb.c tlb.h config.h
	$(CC) $(CFLAGS) -c tlb.c


page_table.o: page_table.c page_table.h
	$(CC) $(CFLAGS) -c page_table.c


process.o: process.c process.h page_table.h
	$(CC) $(CFLAGS) -c process.c


physical_memory.o: physical_memory.c physical_memory.h config.h
	$(CC) $(CFLAGS) -c physical_memory.c


l1_lru.o: l1_lru.c l1_lru.h config.h
	$(CC) $(CFLAGS) -c l1_lru.c


l1.o: l1.c l1.h l1_lru.h config.h bitops.h
	$(CC) $(CFLAGS) -c l1.c


l2_lru.o: l2_lru.c l2_lru.h config.h
	$(CC) $(CFLAGS) -c l2_lru.c


l2.o: l2.c l2.h l2_lru.h config.h bitops.h
	$(CC) $(CFLAGS) -c l2.c


memory_system.o: memory_system.c memory_system.h
	$(CC) $(CFLAGS) -c memory_system.c


clean:
	rm -f *.o simulator comprehensive_test memory_test_report.txt


.PHONY: all clean test
#ifndef CACHE_LATENCY_H
#define CACHE_LATENCY_H

#include <stddef.h>

#define CACHE_LINE_SIZE 64

	typedef struct Node {
		struct Node* next;
		char padding[CACHE_LINE_SIZE - sizeof(struct Node*)];
	} Node;

	Node* create_pointer_chain(size_t size);
	double measure_data_latency(Node* start, int iterations);
	void generate_jump_chain(char* buffer, size_t size);
	double measure_instruction_latency(size_t code_size);
	void test_data_cache();
	void test_instruction_cache();
	void cache_latency_bm(void);

#endif

#ifndef CACHE_BANDWIDTH_H
#define CACHE_BANDWIDTH_H

#include <stdint.h>

	//static inline uint64_t rdtsc_overhead(void);
	static inline uint64_t rdtsc_overhead(void);
	void test_read(size_t size, double *result);
	void test_write(size_t size, double *result);
	void test_copy(size_t size, double *result);
	void test_l1i(double *result);
	void cache_bandwidth_bm(void); 
	
	

#endif

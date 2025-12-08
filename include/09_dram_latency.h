#ifndef DRAM_LATENCY_H
#define DRAM_LATENCY_H

	void create_pointer_chase(void** array, size_t num_elements, size_t stride);
	double measure_dram_latency(void** array, size_t iterations);
	void dram_latency_bm(void);

#endif

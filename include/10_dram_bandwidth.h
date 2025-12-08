#ifndef DRAM_BANDWIDTH_H
#define DRAM_BANDWIDTH_H

	double get_time_seconds();
	double test_write_bandwidth(char *buffer, size_t size);
	double test_read_bandwidth(char *buffer, size_t size);
	double test_copy_bandwidth(char *src, char *dst, size_t size); 
	void dram_bandwidth_bm(void);

#endif

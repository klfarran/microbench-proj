#ifndef SMT_CONTENTION_H
#define SMT_CONTENTION_H
#include <stdint.h>

	typedef struct {
	   int thread_id;
	   int cpu_id;
	   double execution_time;
	} thread_args_t;
	
	/*double get_time();
	void bind_to_cpu(int cpu_id);
	void* vector_compute(void* arg);
	void* cache_thrash(void* arg);
	void run_test(const char* name, void* (*func1)(void*), void* (*func2)(void*), int cpu1, int cpu2); */
	double cycles_to_seconds(uint64_t cycles);
	void bind_to_cpu(int cpu_id);
	void* vector_compute(void* arg);
	void* cache_thrash(void* arg);
	double get_baseline_time(void* (*func)(void*), int cpu);
	void run_test(const char* name, void* (*func1)(void*), void* (*func2)(void*), int cpu1, int cpu2);
	void smt_contention_bm(void);

#endif

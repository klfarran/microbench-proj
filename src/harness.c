//Kelly Farran and Mohan Zhao
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sched.h> //For CPU pinning

#include "00_function_call.h"
#include "01_context_switch.h"
#include "02_fetch_throughput.h"
#include "03_retire_throughput.h"
#include "04_load_store_throughput.h"
#include "05_branch_mispredict.h"
#include "06_exec_unit_throughput.h"
#include "07_cache_latency.h"
#include "08_cache_bandwidth.h"
#include "09_dram_latency.h"
#include "10_dram_bandwidth.h"
#include "11_smt_contention.h"
#include "config.h"

int main(int argc, char** argv){
    /* Setup code -- e.g., CPU pinning, disabling features, etc. */
		
	//Pin CPU
	cpu_set_t cpuset;
	CPU_ZERO(&cpuset); //clear cpuset
	CPU_SET(0, &cpuset); //add cpu 0 to cpuset
	sched_setaffinity(0, sizeof(cpu_set_t), &cpuset); //passing 0 as first argument = current thread 
	
	

    /* Running microbenchmarks and generating results. */
	
	//microbenchmark to measure function call overhead
	function_overhead_bm();
	
	//microbenchmarks to measure context switch overhead
	unsigned long long syscall_cycles = measure_syscall_overhead();
    printf("Average system call (getpid) overhead: %llu cycles\n", syscall_cycles);

    unsigned long long ctxswitch_cycles = measure_thread_switch_overhead();
    printf("Average thread context switch overhead: %llu cycles\n\n", ctxswitch_cycles);
	printf("--------------------------------------------\n\n");
	
	//microbenchmark to measure instruction fetch throughput
	fetch_throughput_bm();
	
	//microbenchmark to measure effective instruction throughput 
	retire_throughput_bm();
	
	//microbenchmark to measure the load/store service rate 
	load_store_throughput_bm();
	
	//microbenchmark to measure the branch misprediction penalty 
	branch_mispredict_bm();
	
	//microbenchmark to measure integer execution unit bandwidth
	exec_unit_throughput_bm();
	
	//microbenchmark to measure cache latencies 
	cache_latency_bm();
	
	//microbenchmark to measure cache bandwidth 
	cache_bandwidth_bm();
	
	//microbenchmark to measure main memory (DRAM) latency 
	dram_latency_bm();
	
	//microbenchmark to measure main memory (DRAM) bandwidth 
	dram_bandwidth_bm();
	
	//microbenchmark to measure SMT contention & symbiosis
	smt_contention_bm();

}

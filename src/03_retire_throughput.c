// Kelly Farran and Mohan Zhao
// Instruction Retire Throughput Microbenchmark
// CS 6354 Microbenchmarking Project

#include "03_retire_throughput.h"
#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h> 

#define NUM_ITER 2000000UL   // number of loop iterations
#define UNROLL   16UL        // unrolled independent operations per iteration
#define CPUID() asm volatile ("CPUID" : : : "%rax", "%rbx", "%rcx", "%rdx", "memory");
#define RDTSCP(cycles) __asm__ volatile ("rdtsc" : "=a" (cycles));

	// measure base rdtsc cost
	static inline uint64_t measure_rdtsc_overhead() {
		uint64_t sum = 0, start = 0, end = 0;
		unsigned int aux;
		
		for (int i = 0; i < 1000; i++) {
			CPUID();
			start = __rdtscp(&aux);
			end = __rdtscp(&aux);
			CPUID();
			sum += (end - start);
		}
		return sum / 1000;
	}

	void retire_throughput_bm(void) {	
	
		uint64_t ETbase = measure_rdtsc_overhead();

		volatile uint64_t a=1,b=2,c=3,d=4,e=5,f=6,g=7,h=8,i=9,j=10,k=11,l=12,m=13;
		uint64_t t0, t1;
		double ipc_low, ipc_med, ipc_high;
		uint64_t start = 0, end = 0;
		int arr[10] = {0};
		unsigned int aux;

		// -------------------------------
		// Case 1: Low ILP (fully dependent chain)
		// -------------------------------
		CPUID();
		start = __rdtscp(&aux);
		for (uint64_t i = 0; i < NUM_ITER; i++) {
			// Each depends on previous: serial dependency
			a = a + 1;
			a = a * 2;
			a = a + 3;
			a = a * 4;
		}
		end = __rdtscp(&aux);
		CPUID();
		uint64_t cycles_low = (end-start) - ETbase;
		uint64_t insts_low  = NUM_ITER * 4;
		ipc_low = (double)insts_low / (double)cycles_low;

		// -------------------------------
		// Case 2: Medium ILP (four independent chains)
		// -------------------------------
		CPUID();
		start = __rdtscp(&aux);
		for (uint64_t i = 0; i < NUM_ITER; i++) {
			a = a + 1;      // chain 1
			b = b * 2;      // chain 2
			c = c + 1;
			d = d * 2;
			a = a + 3;      // chain 1
			b = b * 4;      // chain 2
			c = c + 3;
			d = d * 4;
		}
		end = __rdtscp(&aux);
		CPUID();
		uint64_t cycles_med = (end-start) - ETbase;
		uint64_t insts_med  = NUM_ITER * 8;
		ipc_med = (double)insts_med / (double)cycles_med;

		// -------------------------------
		// Case 3: High ILP (many independent ops)
		// -------------------------------
		CPUID();
		start = __rdtscp(&aux);
		for (uint64_t i = 0; i < NUM_ITER; i++) {
			// UNROLL independent instructions of varying types 
			// compiler must not merge -> volatile vars
			a+=1; //ALU port 0/1
			b*=2; //MUL port 1
			c = arr[0]; //AGU port 2/3/7, LOAD port 2/3  
			d+=1;
			arr[1] = a; //STORE- AGU port 2/3/7, store-data unit 4/5
			e+=5; 
			f*=6; 
			g = arr[1]; 
			h+=1;
			arr[2] = b;
			i+=3; 
			j*=4; 
			k = arr[2]; 
			l+=1;
			arr[3] = c;
			m*=9; 
		}
		end = __rdtscp(&aux);
		CPUID();
		uint64_t cycles_high = (end-start) - ETbase;
		uint64_t insts_high  = NUM_ITER * 16;
		ipc_high = (double)insts_high / (double)cycles_high;

		// -------------------------------
		// Summary
		// -------------------------------
		printf("\n\n=== Effective Instruction Throughput (IPC) ===\n");
		printf("NUM_ITER = %lu, UNROLL = %lu\n", NUM_ITER, UNROLL);
		printf("[Low ILP]   dependent chain:   IPC = %.4f\n", ipc_low);
		printf("[Medium ILP] 4 indep chains:   IPC = %.4f\n", ipc_med);
		printf("[High ILP]  %lu indep ops:     IPC = %.4f\n", UNROLL, ipc_high);

		// derive min, median, max
		double min = ipc_low;
		double max = ipc_high;
		double median = ipc_med;
		printf("--------------------------------------------------\n");
		printf("Min IPC = %.4f, Median IPC = %.4f, Max IPC = %.4f\n", min, median, max);

		uint64_t sum = a+b+c+d+e+f+g+h;
		printf("(avoid optimize-out) sum = %llu\n", (unsigned long long)sum);
		printf("\n--------------------------------------------\n\n\n");
		
	}

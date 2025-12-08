// Accurate measurement of instruction fetch throughput - using NOP instructions and performance counters
// Compile: gcc -O2 -march=native accurate_fetch_throughput.c -o fetch_test.exe
// Run: ./fetch_test.exe

#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>
#include "02_fetch_throughput.h"

#define NUM_ITERATIONS 1000000UL
#define NOP_CHAIN_LENGTH 64UL  // Number of NOP instructions per iteration
#define CPUID() asm volatile ("CPUID" : : : "%rax", "%rbx", "%rcx", "%rdx", "memory");
#define RDTSCP(cycles) __asm__ volatile ("rdtsc" : "=a" (cycles));

// NOP chain macro to prevent optimization
#define NOP64() do { \
    __asm__ __volatile__("nop; nop; nop; nop; nop; nop; nop; nop;" \
                         "nop; nop; nop; nop; nop; nop; nop; nop;" \
                         "nop; nop; nop; nop; nop; nop; nop; nop;" \
                         "nop; nop; nop; nop; nop; nop; nop; nop;" \
                         "nop; nop; nop; nop; nop; nop; nop; nop;" \
                         "nop; nop; nop; nop; nop; nop; nop; nop;" \
                         "nop; nop; nop; nop; nop; nop; nop; nop;" \
                         "nop; nop; nop; nop; nop; nop; nop; nop;" :::); \
} while(0)
	
	
	// Measure fetch efficiency for different instruction lengths
	void test_different_instruction_lengths() {
		printf("=== Testing fetch efficiency for different instruction lengths ===\n");
		
		volatile uint64_t result = 0, start1 = 0, end1 = 0, start2 = 0, end2 = 0;
		unsigned int aux;
		
		// Test 1: 1-byte instructions (NOP)
		CPUID()
		start1 = __rdtscp(&aux);
		for (uint64_t i = 0; i < NUM_ITERATIONS; i++) {
			__asm__ __volatile__("nop; nop; nop; nop; nop; nop; nop; nop;" :::);
			__asm__ __volatile__("nop; nop; nop; nop; nop; nop; nop; nop;" :::);
			__asm__ __volatile__("nop; nop; nop; nop; nop; nop; nop; nop;" :::);
			__asm__ __volatile__("nop; nop; nop; nop; nop; nop; nop; nop;" :::);
			__asm__ __volatile__("nop; nop; nop; nop; nop; nop; nop; nop;" :::);
			__asm__ __volatile__("nop; nop; nop; nop; nop; nop; nop; nop;" :::);
			__asm__ __volatile__("nop; nop; nop; nop; nop; nop; nop; nop;" :::);
			__asm__ __volatile__("nop; nop; nop; nop; nop; nop; nop; nop;" :::);
		}
		end1 = __rdtscp(&aux);
		CPUID()
		
		// Test 2: Mixed-length instructions
		CPUID()
		start2 = __rdtscp(&aux);
		for (uint64_t i = 0; i < NUM_ITERATIONS; i++) {
			// Mix different instruction lengths
			__asm__ __volatile__("mov $0x12345678, %%eax" ::: "eax");  // 5 bytes
			__asm__ __volatile__("add $0x10, %%rax" ::: "rax");        // 4 bytes  
			__asm__ __volatile__("nop" :::);                           // 1 byte
			__asm__ __volatile__("mov $0x12345678, %%ebx" ::: "ebx");  // 5 bytes
			__asm__ __volatile__("add $0x10, %%rbx" ::: "rbx");        // 4 bytes
			__asm__ __volatile__("nop" :::);                           // 1 byte
			// Repeat this pattern...
			__asm__ __volatile__("mov $0x12345678, %%ecx" ::: "ecx");
			__asm__ __volatile__("add $0x10, %%rcx" ::: "rcx");
			__asm__ __volatile__("nop" :::);
			__asm__ __volatile__("mov $0x12345678, %%edx" ::: "edx");
			__asm__ __volatile__("add $0x10, %%rdx" ::: "rdx");
			__asm__ __volatile__("nop" :::);
		}
		end2 = __rdtscp(&aux);
		CPUID()
		
		uint64_t cycles1 = end1 - start1;
		uint64_t cycles2 = end2 - start2;
		uint64_t insts1 = NUM_ITERATIONS * 64;  // 64 NOPs per iteration
		uint64_t insts2 = NUM_ITERATIONS * 12;  // 12 mixed instructions per iteration
		
		printf("1-byte instruction (NOP) IPC: %.3f (%.1f instructions/cycle)\n", 
			   (double)insts1/cycles1, (double)insts1/cycles1);
		printf("Mixed-length instruction IPC: %.3f (%.1f instructions/cycle)\n", 
			   (double)insts2/cycles2, (double)insts2/cycles2);
		printf("Optimization guard result: %llu\n", (unsigned long long)result);
	}
	
	// Test the impact of instruction alignment
	void test_alignment_impact() {
		printf("\n=== Testing the impact of instruction alignment on fetch efficiency ===\n");
		
		// Test different alignments by inserting different numbers of NOPs
		volatile uint64_t results[4] = {0};
		volatile uint64_t start = 0, end = 0;
		unsigned int aux;		

		for (int align = 0; align < 4; align++) {
			CPUID()
			start = __rdtscp(&aux); 
			
			for (uint64_t i = 0; i < NUM_ITERATIONS/10; i++) {
				// Insert NOPs to change alignment
				for (int a = 0; a < align; a++) {
					__asm__ __volatile__("nop" :::);
				}
				
				// Test instruction block
				__asm__ __volatile__("mov %%rax, %%rbx; add $1, %%rax; add $1, %%rbx" ::: "rax", "rbx");
				__asm__ __volatile__("mov %%rcx, %%rdx; add $1, %%rcx; add $1, %%rdx" ::: "rcx", "rdx");
				__asm__ __volatile__("mov %%rsi, %%rdi; add $1, %%rsi; add $1, %%rdi" ::: "rsi", "rdi");
				
				results[align] += i;
			}
			
			end = __rdtscp(&aux);
			CPUID()
			
			uint64_t cycles = end - start;
			uint64_t insts = (NUM_ITERATIONS/10) * (3 + align);  // 3 mov/add + alignment NOPs
			
			printf("Alignment %d bytes: IPC = %.3f\n", align, (double)insts/cycles);
		}
	}
	
	// Main test function
	void test_max_fetch_throughput() {
		printf("=== Maximum fetch throughput test (using NOP instructions) ===\n");
		
		volatile uint64_t dummy = 0, start = 0, end = 0;
		unsigned int aux;
		
		// Measure RDTSC overhead
		uint64_t tsc_overhead = 0;
		for (int i = 0; i < 1000; i++) {
			CPUID()
			start = __rdtscp(&aux); 
			end = __rdtscp(&aux);
			CPUID()
			tsc_overhead += (end - start);
		}
		tsc_overhead /= 1000;
		
		// Warm-up
		for (int w = 0; w < 1000; w++) {
			NOP64();
			dummy += w;
		}
		
		// Main measurement
		CPUID()
		start = __rdtscp(&aux);
		
		for (uint64_t iter = 0; iter < NUM_ITERATIONS; iter++) {
			// Measure fetch throughput using pure NOPs
			NOP64();  // 64 NOPs per iteration
		}
		
		end = __rdtscp(&aux);
		CPUID()
		
		uint64_t cycles = (end - start) - tsc_overhead;
		uint64_t total_insts = NUM_ITERATIONS * NOP_CHAIN_LENGTH;
		
		double ipc = (double)total_insts / cycles;
		double fetch_width = ipc;  // For NOPs, IPC ≈ fetch width
		
		printf("Test results:\n");
		printf("Total cycles: %llu\n", (unsigned long long)cycles);
		printf("Total instructions: %llu\n", (unsigned long long)total_insts);
		printf("Instructions per cycle (IPC): %.4f\n", ipc);
		printf("Estimated fetch width: %.2f instructions/cycle\n", fetch_width);
		printf("Optimization guard value: %llu\n", (unsigned long long)dummy);
		
		// Estimate CPU fetch width based on IPC
		if (fetch_width >= 5.5) printf("Inference: 6-wide fetch (modern high-end CPU)\n");
		else if (fetch_width >= 3.5) printf("Inference: 4-wide fetch (mainstream CPU)\n");
		else if (fetch_width >= 2.5) printf("Inference: 3-wide fetch (older CPU)\n");
		else printf("Inference: 2-wide or narrower fetch\n");
	}

	void fetch_throughput_bm(void) { 
		printf("===Instruction Fetch Throughput===\n\n");
		test_max_fetch_throughput();
		printf("\n");
		test_different_instruction_lengths();
		printf("\n");
		test_alignment_impact();
		printf("\n--------------------------------------------\n");
	}	

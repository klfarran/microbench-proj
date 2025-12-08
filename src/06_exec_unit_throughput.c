// Kelly Farran and Mohan Zhao
// Executive Unit Throughput Benchmark
// CS 6354 Microbenchmarking Project

#include "06_exec_unit_throughput.h"
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <x86intrin.h>

#define NUM_ITER (1000000UL)
#define UNROLL (16UL)  // 减少展开，确保寄存器足够
//#define UNROLL100(X) X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X
#define UNROLL50(X) X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X
#define CPUID() asm volatile ("CPUID" : : : "%rax", "%rbx", "%rcx", "%rdx", "memory");

	static inline uint64_t rdtsc_overhead(void) {
		uint64_t s,e,sum=0;
		unsigned int aux;
		for (int i=0;i<1000;i++){
			CPUID()
			uint64_t s = __rdtscp(&aux);
			uint64_t e = __rdtscp(&aux);
			CPUID()
			sum += (e - s);
		}
		return sum / 1000;
	}
	
	// 修复的ALU测试：完全独立的操作
	void test_alu_fixed(uint64_t iterations, uint64_t unroll, 
					   uint64_t *cycles, uint64_t *insts) {
		// 使用更多独立变量，确保无依赖
		uint64_t v0=1, v1=2, v2=3, v3=4, v4=5, v5=6, v6=7, v7=8;
		uint64_t v8=9, v9=10, v10=11, v11=12, v12=13, v13=14, v14=15, v15=16;
		
		volatile uint64_t start, end, total = 0;
		volatile uint64_t ETbase = rdtsc_overhead();
		unsigned int aux;

		start = __rdtscp(&aux);
		
		for (uint64_t it = 0; it < iterations; it++) {
			// 完全独立的加法操作，无依赖链
			UNROLL50({
			v0 += 1;  v1 += 1;  v2 += 1;  v3 += 1;
			v4 += 1;  v5 += 1;  v6 += 1;  v7 += 1;
			v8 += 1;  v9 += 1; v10 += 1; v11 += 1;
			v12 += 1; v13 += 1; v14 += 1; v15 += 1;
			});		
		}

		end = __rdtscp(&aux);
		
		//*cycles = (end - start > ETbase) ? (end - start - ETbase) : (end - start);
		*cycles = (end-start) - ETbase;
		*insts =  iterations * unroll * 50;
	}

	// 修复的乘法测试
	void test_mul_fixed(uint64_t iterations, uint64_t unroll,
					   uint64_t *cycles, uint64_t *insts) {
		volatile uint64_t v0=1001, v1=1002, v2=1003, v3=1004;
		volatile uint64_t v4=1005, v5=1006, v6=1007, v7=1008;
		volatile uint64_t v8=1009, v9=1010, v10=1011, v11=1012;
		volatile uint64_t v12=1013, v13=1014, v14=1015, v15=1016;
		
		uint64_t start, end;
		uint64_t ETbase = rdtsc_overhead();
		unsigned int aux;

		CPUID()
		start = __rdtscp(&aux);
		
		for (uint64_t it = 0; it < iterations; ++it) {
			// 完全独立的乘法操作
			v0 *= 3;  v1 *= 5;  v2 *= 7;  v3 *= 11;
			v4 *= 13; v5 *= 17; v6 *= 19; v7 *= 23;
			v8 *= 29; v9 *= 31; v10 *= 37; v11 *= 41;
			v12 *= 43; v13 *= 47; v14 *= 53; v15 *= 59;
		}
		
		end = __rdtscp(&aux);
		CPUID()
		
		*cycles = (end - start > ETbase) ? (end - start - ETbase) : (end - start);
		*insts = iterations * unroll;
	}

	// 修复的除法测试
	void test_div_fixed(uint64_t iterations, uint64_t unroll,
					   uint64_t *cycles, uint64_t *insts) {
		volatile uint64_t v0=1000000001, v1=1000000002, v2=1000000003, v3=1000000004;
		volatile uint64_t v4=1000000005, v5=1000000006, v6=1000000007, v7=1000000008;
		
		uint64_t start, end;
		uint64_t ETbase = rdtsc_overhead();
		unsigned int aux;

		CPUID()
		start = __rdtscp(&aux);
		
		for (uint64_t it = 0; it < iterations; ++it) {
			// 完全独立的除法操作
			v0 /= 3;  v1 /= 5;  v2 /= 7;  v3 /= 11;
			v4 /= 13; v5 /= 17; v6 /= 19; v7 /= 23;
		}
		
		end = __rdtscp(&aux);
		CPUID()
		
		*cycles = (end - start > ETbase) ? (end - start - ETbase) : (end - start);
		*insts = iterations * unroll;
	}

	void exec_unit_throughput_bm(void) {	
		const uint64_t ITER = NUM_ITER;
		const uint64_t UNR = UNROLL;
		
		uint64_t ETbase = rdtsc_overhead();
		//printf("RDTSC overhead: %llu cycles\n", (unsigned long long)ETbase);

		printf("=== Integer Execution Unit Bandwidth (Fixed) ===\n");
		printf("Testing with completely independent operations...\n\n");

		// 运行修复的测试
		uint64_t cycles_alu, insts_alu, cycles_mul, insts_mul, cycles_div, insts_div;
		
		test_alu_fixed(ITER, UNR, &cycles_alu, &insts_alu);
		test_mul_fixed(ITER, UNR, &cycles_mul, &insts_mul);
		test_div_fixed(ITER/4, 8, &cycles_div, &insts_div);  // 除法用更少操作

		double ops_cycle_alu = (double)insts_alu / (double)cycles_alu;
		double ops_cycle_mul = (double)insts_mul / (double)cycles_mul;
		double ops_cycle_div = (double)insts_div / (double)cycles_div;

		printf("NUM_ITER = %" PRIu64 ", UNROLL = %" PRIu64 "\n", ITER, UNR);
		printf("[Simple ALU add]   total insts = %" PRIu64 ", cycles = %" PRIu64 ", ops/cycle = %.6f\n",
			   insts_alu, cycles_alu, ops_cycle_alu);
		printf("[Integer MUL]      total insts = %" PRIu64 ", cycles = %" PRIu64 ", ops/cycle = %.6f\n",
			   insts_mul, cycles_mul, ops_cycle_mul);
		printf("[Integer DIV]      total insts = %" PRIu64 ", cycles = %" PRIu64 ", ops/cycle = %.6f\n",
			   insts_div, cycles_div, ops_cycle_div);
			   
		printf("\n--------------------------------------------\n\n");
	}

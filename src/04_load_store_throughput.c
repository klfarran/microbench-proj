// Kelly Farran and Mohan Zhao
// Load Store Throughput Microbenchmark
// CS 6354 Microbenchmarking Project

#include "04_load_store_throughput.h"
#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

#define N 32768         // array length (~256 KB -> fits in L1/L2)
#define REPEAT 1000000  // loop iterations
#define CPUID() asm volatile ("CPUID" : : : "%rax", "%rbx", "%rcx", "%rdx", "memory");
#define RDTSCP(cycles) __asm__ volatile ("rdtsc" : "=a" (cycles));

	static inline uint64_t measure_rdtsc_overhead() {
		uint64_t sum = 0, start = 0, end = 0;
		unsigned int aux;

		for (int i = 0; i < 1000; i++) {
			CPUID()
			start = __rdtscp(&aux);
			end = __rdtscp(&aux);
			CPUID()
			sum += (end - start);
		}
		return sum / 1000;
	}

	void load_store_throughput_bm(void) {	
		
    static double A[N], B[N], C[N], D[N], E[N], F[N], G[N], H[N];
    
    for (int i = 0; i < N; i++) {
        A[i] = i * 1.0;
        B[i] = i * 1.0;
        C[i] = i * 1.0;
	D[i] = i * 1.0;
	E[i] = i * 1.0;
	F[i] = i * 1.0;
	G[i] = i * 1.0;
	H[i] = i * 1.0;
    }

    uint64_t ETbase = measure_rdtsc_overhead();
    uint64_t t0, t1;
    unsigned int aux;
    int idx = 0;
    
    printf("=== Load/Store Service Rate (Fixed) ===\n");
    printf("Testing different dependency patterns...\n\n");

    // ------------------------------------------
    // Case 1: ORIGINAL LOAD-heavy (with dependency chain)
    // ------------------------------------------
    double sum = 0.0;
    uint64_t cycles_load_dep = 0;
    CPUID()
    
    for (int r = 0; r < REPEAT; r++) {
	idx+=8;
	if(idx > N - 8) idx = 0; //prevent segfaults

        t0 = __rdtscp(&aux);
	 sum+= A[0];
	 sum+= A[1];
	 sum+= A[2];
	 sum+= A[3];
	 sum+= A[4];
	 sum+= A[5];
	 sum+= A[6];
	 sum+= A[7];
        t1 = __rdtscp(&aux);
        cycles_load_dep += (t1-t0) - ETbase;
    }
    CPUID()
    double load_ipc_dep = (double)(REPEAT * 8) / (double)cycles_load_dep;

    // ------------------------------------------
    // Case 2: FIXED LOAD-heavy (no dependency chain)
    // ------------------------------------------
    double sum1 = 0.0, sum2 = 0.0, sum3 = 0.0, sum4 = 0.0;
    double sum5 = 0.0, sum6 = 0.0, sum7 = 0.0, sum8 = 0.0;
    double sum9 = 0.0, sum10 = 0.0, sum11 = 0.0, sum12 = 0.0;    
    uint64_t cycles_load_indep = 0;
    idx = 0;
 
    CPUID()
    
    for (int r = 0; r < REPEAT; r++) {

	if(idx >= N) idx =0;

	//avoid cache miss:
	//sum+= A[idx] + B[idx] + C[idx] + D[idx] + E[idx] + F[idx] + G[idx] + H[idx];
		
	sum+=A[1]+B[2]+C[3]+D[4]+E[5]+F[6]+G[7]+H[8];

      t0 = __rdtscp(&aux);
	sum1= A[1];
	sum2= B[2];
	sum3= C[3];
	sum4= D[4];
	sum5= E[5];
	sum6= F[6];
	sum7= G[7];
	sum8= H[8];
	
     t1 = __rdtscp(&aux);
	sum+= sum1+sum2+sum3+sum4+sum5+sum6+sum7+sum8;     

	idx++;

     cycles_load_indep += (t1-t0) - ETbase;
    }
    
    double load_ipc_indep = (double)(REPEAT * 8) / (double)cycles_load_indep;
    CPUID()
    // 合并结果防止优化
    sum += sum1 + sum2 + sum3 + sum4 + sum5 + sum6 + sum7 + sum8;

    // ------------------------------------------
    // Case 3: ORIGINAL STORE- no dependency chain
    // ------------------------------------------
    
    CPUID() 
    uint64_t cycles_store_indep = 0;
    idx = 0;
    
    for (int r = 0; r < REPEAT; r++) {
	idx+=8;
	if(idx > N - 8) idx = 0;

	sum+=A[0]+B[1]+C[2]+D[3]+E[4]+F[5]+G[6]+H[7];

        t0 = __rdtscp(&aux);
        A[0] = sum;
        B[1] = sum;
        C[2] = sum;
        D[3] = sum;
        E[4] = sum;
        F[5] = sum;
        H[6] = sum;
        G[7] = sum;
        t1 = __rdtscp(&aux);
       cycles_store_indep += (t1-t0) - ETbase;
    }
    
    double store_ipc = (double)(REPEAT * 8) / (double)cycles_store_indep;
    CPUID()
    // ------------------------------------------
    // Case 4: STORE with dependency chain (对比实验)
    // ------------------------------------------
    double val = 1.0;
    uint64_t cycles_store_dep = 0;
    idx = 0;

    CPUID()
    
    for (int r = 0; r < REPEAT; r++) {
	idx+=8;
	if(idx > N - 8) idx =0;

        t0 = __rdtscp(&aux);
        C[idx] = val;
	val++;
        C[idx+1] = val;
	val++;
        C[idx+2] = val;
	val++;
        C[idx+3] = val;
        val++;
	C[idx+4] = val;
        val++;
	C[idx+5] = val;
        val++;
	C[idx+6] = val;
        val++;
	C[idx+7] = val;
    	t1 = __rdtscp(&aux);
	cycles_store_dep += (t1-t0) - ETbase;
	}
    
	CPUID()
    
    double store_ipc_dep = (double)(REPEAT * 8) / (double)cycles_store_dep;

    // ------------------------------------------
    // Summary
    // ------------------------------------------
    printf("=== RESULTS ===\n");
    printf("Array size: %d doubles (%.1f KB)\n", N, N * sizeof(double) / 1024.0);
    printf("REPEAT = %d\n\n", REPEAT);
    
    printf("[LOAD - WITH DEPENDENCY]\n");
    printf("  cycles = %llu, loads/cycle = %.4f\n", 
           (unsigned long long)cycles_load_dep, load_ipc_dep);
    
    printf("[LOAD - NO DEPENDENCY]\n");
    printf("  cycles = %llu, loads/cycle = %.4f\n", 
           (unsigned long long)cycles_load_indep, load_ipc_indep);
    
    printf("[STORE - NO DEPENDENCY]\n");
    printf("  cycles = %llu, stores/cycle = %.4f\n", 
           (unsigned long long)cycles_store_indep, store_ipc);
    
    printf("[STORE - WITH DEPENDENCY]\n");
    printf("  cycles = %llu, stores/cycle = %.4f\n", 
           (unsigned long long)cycles_store_dep, store_ipc_dep);
    
    printf("\n=== ANALYSIS ===\n");
    printf("Load speedup (no dep vs dep): %.2fx\n", load_ipc_indep / load_ipc_dep);
    printf("Store slowdown (dep vs no dep): %.2fx\n", store_ipc / store_ipc_dep);
    
    if (store_ipc > load_ipc_indep) {
        printf("CONCLUSION: Stores are genuinely faster than loads on this CPU\n");
    } else {
        printf("CONCLUSION: The 'faster stores' was due to dependency chains\n");
    }

    double result = sum + B[0] + C[0];
    printf("\n(prevent optimize-out) = %.2f\n", result);
	printf("\n--------------------------------------------\n\n");
	
	}

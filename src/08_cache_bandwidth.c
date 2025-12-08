#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <x86intrin.h>
#include "config.h"

#define KB (1024ULL)
#define MB (1024ULL * KB)
#define CPUID asm volatile ("CPUID" : : : "%rax", "%rbx", "%rcx", "%rdx", "memory");
#define ITERS (50000ULL)

static inline uint64_t rdtsc_overhead(void) {
		uint64_t s,e,sum=0;
		unsigned int aux;
		for (int i=0;i<1000;i++){
			CPUID
			uint64_t s = __rdtscp(&aux);
			uint64_t e = __rdtscp(&aux);
			CPUID
			sum += (e - s);
		}
		return sum / 1000;
	}

// 读测试 - 多累加器展开
void test_read(size_t size, double *result) {
    double *a = aligned_alloc(64, size);
	unsigned int aux;
    if (!a) { *result = 0; return; }
    
    size_t n = size / sizeof(double);
    for (size_t i = 0; i < n; i++) a[i] = (double)i;
    
    double s0 = 0, s1 = 0, s2 = 0, s3 = 0;
    double s4 = 0, s5 = 0, s6 = 0, s7 = 0;
    
    // 预热
    for (int k = 0; k < 10; k++) {
        for (size_t i = 0; i < n; i += 8) {
            s0 += a[i];   s1 += a[i+1];
            s2 += a[i+2]; s3 += a[i+3];
            s4 += a[i+4]; s5 += a[i+5];
            s6 += a[i+6]; s7 += a[i+7];
        }
    }
    
    // 测量
    CPUID 
	uint64_t start = __rdtscp(&aux);
    int iters = 0;
    
    for (long long iter = 0; iter < ITERS; iter++) {
        for (size_t i = 0; i < n; i += 8) {
            s0 += a[i];   s1 += a[i+1];
            s2 += a[i+2]; s3 += a[i+3];
            s4 += a[i+4]; s5 += a[i+5];
            s6 += a[i+6]; s7 += a[i+7];
        }
    } 
    
	uint64_t end = __rdtscp(&aux);
	CPUID 
	
    double elapsed = end - start - rdtsc_overhead();
    volatile double sum = s0+s1+s2+s3+s4+s5+s6+s7;
    if (sum == -1.0) printf("%f", sum);
    
    long long total_bytes = (long long)size * ITERS;
    *result = ((double)total_bytes / elapsed) * cpu_freq_ghz;
    free(a);
}

// 写测试 - 展开循环
void test_write(size_t size, double *result) {
    double *a = aligned_alloc(64, size);
    if (!a) { *result = 0; return; }
    
    size_t n = size / sizeof(double);
    memset(a, 0, size);
	unsigned int aux;
    
    // 预热
    for (int k = 0; k < 10; k++) {
        for (size_t i = 0; i < n; i += 8) {
            a[i] = 3.0;   a[i+1] = 3.0;
            a[i+2] = 3.0; a[i+3] = 3.0;
            a[i+4] = 3.0; a[i+5] = 3.0;
            a[i+6] = 3.0; a[i+7] = 3.0;
        }
    }
    
    // 测量
	CPUID
    uint64_t start = __rdtscp(&aux);
    
    for (long long iter = 0; iter < ITERS; iter++) {
        for (size_t i = 0; i < n; i += 8) {
            a[i] = 3.0;   a[i+1] = 3.0;
            a[i+2] = 3.0; a[i+3] = 3.0;
            a[i+4] = 3.0; a[i+5] = 3.0;
            a[i+6] = 3.0; a[i+7] = 3.0;
        }
    } 
    
	uint64_t end = __rdtscp(&aux);
	CPUID
	
    double elapsed = end - start - rdtsc_overhead();
    long long total_bytes = (long long)size * ITERS;
    *result = ((double)total_bytes / elapsed) * cpu_freq_ghz;
    free(a);
}

// 拷贝测试 - 展开循环
void test_copy(size_t size, double *result) {
    double *a = aligned_alloc(64, size);
    double *b = aligned_alloc(64, size);
	unsigned int aux;
	
    if (!a || !b) { *result = 0; return; }
    
    size_t n = size / sizeof(double);
    for (size_t i = 0; i < n; i++) {
        a[i] = 1.0;
        b[i] = 2.0;
    }
    
    // 预热
    for (int k = 0; k < 10; k++) {
        for (size_t i = 0; i < n; i += 8) {
            b[i] = a[i];     b[i+1] = a[i+1];
            b[i+2] = a[i+2]; b[i+3] = a[i+3];
            b[i+4] = a[i+4]; b[i+5] = a[i+5];
            b[i+6] = a[i+6]; b[i+7] = a[i+7];
        }
    }
    
    // 测量
    CPUID
    uint64_t start = __rdtscp(&aux);
    
    for (long long iter = 0; iter < ITERS; iter++) {
        for (size_t i = 0; i < n; i += 8) {
            b[i] = a[i];     b[i+1] = a[i+1];
            b[i+2] = a[i+2]; b[i+3] = a[i+3];
            b[i+4] = a[i+4]; b[i+5] = a[i+5];
            b[i+6] = a[i+6]; b[i+7] = a[i+7];
        }
    } 
    
	uint64_t end = __rdtscp(&aux);
	CPUID
	
    double elapsed = end - start - rdtsc_overhead();
    long long total_bytes = (long long)size * ITERS;
    *result = ((double)total_bytes / elapsed) * cpu_freq_ghz;
	
    free(a);
    free(b);
}

// L1I测试 - 用内联汇编防止优化
void test_l1i(double *result) {
    volatile uint64_t dummy = 0;
	unsigned int aux;
    
    // 预热
    for (int i = 0; i < 100; i++) {
        dummy++;
    }
    
    // 测量
	CPUID
    uint64_t start = __rdtscp(&aux);
    
    for (long long iter = 0; iter < ITERS / 10; iter++) {
        for (int i = 0; i < 10000; i++) {
            // 使用汇编指令，每条4字节
            __asm__ volatile (
                "add $1, %%rax\n\t"
                "add $1, %%rbx\n\t"
                "add $1, %%rcx\n\t"
                "add $1, %%rdx\n\t"
                "add $1, %%rax\n\t"
                "add $1, %%rbx\n\t"
                "add $1, %%rcx\n\t"
                "add $1, %%rdx\n\t"
                "add $1, %%rax\n\t"
                "add $1, %%rbx\n\t"
                "add $1, %%rcx\n\t"
                "add $1, %%rdx\n\t"
                "add $1, %%rax\n\t"
                "add $1, %%rbx\n\t"
                "add $1, %%rcx\n\t"
                "add $1, %%rdx\n\t"
                : 
                : 
                : "rax", "rbx", "rcx", "rdx"
            );
        }
    }
    
	uint64_t end = __rdtscp(&aux);
    CPUID;
	
    double elapsed = end - start - rdtsc_overhead();
	double elapsed_time = (double)elapsed / (cpu_freq_ghz * 1e9);
    
    double instr_bytes = 10000.0 * 16 * 4 * (ITERS / 10); // 16条指令，每条4字节
    *result = (instr_bytes / elapsed) * cpu_freq_ghz;
}

void cache_bandwidth_bm(void) {
    printf("\n===Cache Bandwidth Benchmark===\n");
    
    struct {
        const char *name;
        size_t size;
    } tests[] = {
        /*{"L1D", 24 * KB},   // 减小到24KB，确保在L1内
        {"L2",  192 * KB},  // 减小到192KB，确保在L2内
        {"L3",  6 * MB}  */   // 6MB
		{"L1D", 24 * KB},   
        {"L2",  256 * KB},  
        {"L3",  6 * MB}     
    };
    
    for (int i = 0; i < 3; i++) {
        printf("%s Cache (%zu KB):\n", tests[i].name, tests[i].size / KB);
        
        double r, w, c;
        test_read(tests[i].size, &r);
        printf("  Read:  %6.2f GB/s\n", r);
        
        test_write(tests[i].size, &w);
        printf("  Write: %6.2f GB/s\n", w);
        
        test_copy(tests[i].size, &c);
        printf("  Copy:  %6.2f GB/s\n", c);
        printf("\n");
    }
    
    printf("L1I (Instruction Cache):\n");
    double l1i;
    test_l1i(&l1i);
    printf("  Fetch: %6.2f GB/s\n\n", l1i);
    
    printf("Note: Results should show L1D > L2 > L3\n");
    
    return 0;
}

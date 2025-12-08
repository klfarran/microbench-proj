#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sched.h>
#include <sys/time.h>
#include <unistd.h>
#include <immintrin.h>
#include <stdint.h>
#include <inttypes.h>
#include <x86intrin.h>
#include "config.h"

#define ARRAY_SIZE (64 * 1024 * 1024)
#define ITERS 200000000L
#define CPUID __asm__ __volatile__ ("CPUID" : : : "%rax", "%rbx", "%rcx", "%rdx", "memory");

//Synchronization and Timing 
pthread_barrier_t contention_barrier;

typedef struct {
    int thread_id;
    int cpu_id;
    uint64_t execution_cycles; // Time measured in cycles
} thread_args_t;

// Function to convert cycles to seconds
double cycles_to_seconds(uint64_t cycles) {
    return (double)cycles / (cpu_freq_ghz * 1e9);
}

void bind_to_cpu(int cpu_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_id, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
}


void* vector_compute(void* arg) {
    thread_args_t* args = (thread_args_t*)arg;
    bind_to_cpu(args->cpu_id);
    unsigned int aux; 
    
    // Allocate memory for vector operations
    double* a = (double*)aligned_alloc(32, 1024 * sizeof(double));
    double* b = (double*)aligned_alloc(32, 1024 * sizeof(double));
    double* c = (double*)aligned_alloc(32, 1024 * sizeof(double));
    
    // Initialization
    for (int i = 0; i < 1024; i++) {
        a[i] = i * 0.001;
        b[i] = i * 0.002;
    }
    
    // Wait for the other thread to synchronize
    pthread_barrier_wait(&contention_barrier);
    
	
    CPUID
    uint64_t start_cycles = __rdtscp(&aux); 
    
    // Main Compute Loop
    for (long iter = 0; iter < ITERS / 1000; iter++) {
        for (int i = 0; i < 1024; i += 4) {
            __m256d va = _mm256_load_pd(&a[i]);
            __m256d vb = _mm256_load_pd(&b[i]);
            __m256d vc = _mm256_mul_pd(va, vb);
            vc = _mm256_add_pd(vc, va);
            vc = _mm256_div_pd(vc, vb);
            vc = _mm256_sqrt_pd(vc);
            _mm256_store_pd(&c[i], vc);
            a[i] = c[i] * 0.9;
            b[i] = c[i] * 1.1;
        }
    }
    
    uint64_t end_cycles = __rdtscp(&aux);
    CPUID
    
    args->execution_cycles = end_cycles - start_cycles;
    
    volatile double sum = 0;
    for (int i = 0; i < 10; i++) sum += c[i];
    
    free(a); free(b); free(c);
    return NULL;
}

void* cache_thrash(void* arg) {
    thread_args_t* args = (thread_args_t*)arg;
    bind_to_cpu(args->cpu_id);
    unsigned int aux; 
    
	
    // Allocate large array for memory access
    int* array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    srand(time(NULL) + args->thread_id);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = rand();
    }
    
    // Wait for the other thread to synchronize
    pthread_barrier_wait(&contention_barrier);
    
    CPUID 
    uint64_t start_cycles = __rdtscp(&aux);
    
    // Main Memory Thrash Loop
    volatile long long sum = 0;
    for (long i = 0; i < ITERS / 10; i++) {
        int idx1 = (i * 7919) % ARRAY_SIZE;
        int idx2 = (i * 104729) % ARRAY_SIZE;
        int idx3 = (i * 524287) % ARRAY_SIZE;
        sum += array[idx1];
        array[idx2] ^= (int)sum;
        sum += array[idx3];
        array[idx1] = (int)(sum & 0xFFFFFF);
    }
    
    uint64_t end_cycles = __rdtscp(&aux);
    CPUID
    
    args->execution_cycles = end_cycles - start_cycles;
    
    if (sum == 0) printf("");
    
    free(array);
    return NULL;
	
}


// function to get a single-threaded baseline time for efficiency calculation
double get_baseline_time(void* (*func)(void*), int cpu) {
    thread_args_t args = {1, cpu, 0};
    
    // temporarily initialize barrier for a single thread
    pthread_barrier_init(&contention_barrier, NULL, 1);
    
    pthread_t t;
    pthread_create(&t, NULL, func, &args);
    pthread_join(t, NULL);
    
    pthread_barrier_destroy(&contention_barrier);
    
    return cycles_to_seconds(args.execution_cycles);
}


void run_test(const char* name, void* (*func1)(void*), void* (*func2)(void*), 
              int cpu1, int cpu2) {
    pthread_t t1, t2;
    thread_args_t args1 = {1, cpu1, 0};
    thread_args_t args2 = {2, cpu2, 0};
    unsigned int aux; 
    
    // 1. measure Single-Thread Baseline (Needed for accurate efficiency calculation)
    double baseline1 = get_baseline_time(func1, cpu1);
    double baseline2 = get_baseline_time(func2, cpu2);
    double ideal_sum_time = baseline1 + baseline2;

    printf("\n%s (CPU %d, %d):\n", name, cpu1, cpu2);
    
    // 2. prepare for Dual-Thread Run
    pthread_barrier_init(&contention_barrier, NULL, 2);
    
    CPUID
    uint64_t start_wall_cycles = __rdtscp(&aux);
    pthread_create(&t1, NULL, func1, &args1);
    pthread_create(&t2, NULL, func2, &args2);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    uint64_t end_wall_cycles = __rdtscp(&aux); 
    CPUID
    
    pthread_barrier_destroy(&contention_barrier);
    
    uint64_t total_cycles = end_wall_cycles - start_wall_cycles;
    double total_wall_time = cycles_to_seconds(total_cycles);
    
    // convert Contended Times to seconds
    double thread1_sec = cycles_to_seconds(args1.execution_cycles);
    double thread2_sec = cycles_to_seconds(args2.execution_cycles);
    
    // efficiency calculation: (Ideal Sum Time) / Wall Time
    double efficiency = (ideal_sum_time / total_wall_time) * 100.0;
    
    printf("  Thread 1 Contended: %.3f sec (Baseline: %.3f sec)\n", thread1_sec, baseline1);
    printf("  Thread 2 Contended: %.3f sec (Baseline: %.3f sec)\n", thread2_sec, baseline2);
    printf("  Wall time: %.3f sec\n", total_wall_time);
    printf("  Efficiency: %.1f%%\n", efficiency);
}

void smt_contention_bm(void) {
    printf("SMT Contention & Symbiosis Test (Inline RDTSCP)\n");
    printf("Clock Frequency set at %.2f GHz\n", cpu_freq_ghz);
    printf("CPUs: %ld\n", sysconf(_SC_NPROCESSORS_ONLN));
    
    // Scenario 1: Compute+Compute (same core)
    run_test("Scenario 1: Compute+Compute (same core)",
             vector_compute, vector_compute, 0, 1);
    
    // Scenario 2: Memory+Memory (same core) 
    run_test("Scenario 2: Memory+Memory (same core)",
             cache_thrash, cache_thrash, 0, 1);
    
    // Scenario 3: Compute+Memory (same core)
    run_test("Scenario 3: Compute+Memory (same core)",
             vector_compute, cache_thrash, 0, 1);
    
    // Scenario 4: Compute+Compute (different cores)
    run_test("Scenario 4: Compute+Compute (different cores)",
             vector_compute, vector_compute, 0, 2);
}

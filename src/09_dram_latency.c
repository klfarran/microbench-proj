#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include "config.h"

#define MB (1024 * 1024)
#define CACHE_LINE_SIZE 64

uint64_t get_time_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

void create_pointer_chase(void** array, size_t num_elements, size_t stride) {
    size_t* indices = malloc(num_elements * sizeof(size_t));
    
    for (size_t i = 0; i < num_elements; i++) {
        indices[i] = i * stride;
    }
    
    // Fisher-Yates shuffle for random access pattern
    for (size_t i = num_elements - 1; i > 0; i--) {
        size_t j = rand() % (i + 1);
        size_t temp = indices[i];
        indices[i] = indices[j];
        indices[j] = temp;
    }
    
    // Create pointer chase chain
    for (size_t i = 0; i < num_elements - 1; i++) {
        array[indices[i] / sizeof(void*)] = (void*)((char*)array + indices[i + 1]);
    }
    array[indices[num_elements - 1] / sizeof(void*)] = (void*)((char*)array + indices[0]);
    
    free(indices);
}

double measure_dram_latency(void** array, size_t iterations) {
    void* ptr = array;
    uint64_t start, end;
    
    // Warmup phase
    for (size_t i = 0; i < 100000; i++) {
        ptr = *(void**)ptr;
    }
    
    // Measurement
    start = get_time_ns();
    
    for (size_t i = 0; i < iterations; i++) {
        ptr = *(void**)ptr;
        ptr = *(void**)ptr;
        ptr = *(void**)ptr;
        ptr = *(void**)ptr;
        ptr = *(void**)ptr;
        ptr = *(void**)ptr;
        ptr = *(void**)ptr;
        ptr = *(void**)ptr;
        ptr = *(void**)ptr;
        ptr = *(void**)ptr;
    }
    
    end = get_time_ns();
    
    if (ptr == (void*)0x1) printf("x");
    
    return (double)(end - start) / (iterations * 10);
}

void dram_latency_bm(void) {
    printf("DRAM Latency Test\n");
    printf("=================\n\n");
    
    // Test sizes large enough to exceed all cache levels
    size_t test_sizes[] = {
        128 * MB,
        256 * MB,
        512 * MB,
        1024 * MB
    };
    
    int num_tests = sizeof(test_sizes) / sizeof(test_sizes[0]);
    size_t stride = CACHE_LINE_SIZE;
    size_t iterations = 100000;
    
    printf("Configuration:\n");
    printf("  Stride: %zu bytes\n", stride);
    printf("  Iterations: %zu\n", iterations);
    printf("  Access pattern: Random pointer chasing\n\n");
    
    printf("Size (MB)    Latency (ns)    Cycles (@%.1fGHz)\n", cpu_freq_ghz);
    printf("----------------------------------------------\n");
    
    for (int i = 0; i < num_tests; i++) {
        size_t size = test_sizes[i];
        size_t num_elements = size / stride;
        
        void** array = aligned_alloc(CACHE_LINE_SIZE, size);
        if (array == NULL) {
            printf("%-12zu Memory allocation failed\n", size / MB);
            continue;
        }
        
        memset(array, 0, size);
        srand(42 + i);
        create_pointer_chase(array, num_elements, stride);
        
        double latency_ns = measure_dram_latency(array, iterations);
        double cycles = latency_ns * cpu_freq_ghz;
        
        printf("%-12zu %.2f          ~%.0f\n", size / MB, latency_ns, cycles);
        
        free(array);
    }
    
    printf("\nNotes:\n");
    printf("- Test uses memory larger than typical L3 cache (8-64MB)\n");
    printf("- Random access pattern prevents hardware prefetching\n");
    printf("- Typical DRAM latency: 60-120ns (DDR4/DDR5)\n");
    
    return 0;
}

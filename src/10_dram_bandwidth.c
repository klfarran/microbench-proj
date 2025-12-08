#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#define BUFFER_SIZE (1024 * 1024 * 1024)  // 1 GB
#define ITERATIONS 5

// 防止编译器优化
volatile long long dummy = 0;

double get_time_seconds() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

// 测试持续写带宽
double test_write_bandwidth(char *buffer, size_t size) {
    double start = get_time_seconds();
    
    for (int i = 0; i < ITERATIONS; i++) {
        memset(buffer, i, size);
    }
    
    double end = get_time_seconds();
    double elapsed = end - start;
    double total_gb = (size * ITERATIONS) / (1024.0 * 1024.0 * 1024.0);
    
    return total_gb / elapsed;
}

// 测试持续读带宽
double test_read_bandwidth(char *buffer, size_t size) {
    double start = get_time_seconds();

    char *p = buffer;    
    uint64_t sum = 0;
    size_t num_elements = size/ 64;

    for (int i = 0; i < ITERATIONS; i++) {
	for(size_t j = 0; j < num_elements; j++) {
	   sum+=p[j*64];
	}
        /*for (size_t j = 0; j < size; j+=64) {
            sum += buffer[j];
        }*/
    }
    dummy = sum;  // 防止优化
    
    double end = get_time_seconds();
    double elapsed = end - start;
    double total_gb = (size * ITERATIONS) / (1024.0 * 1024.0 * 1024.0);
    
    return total_gb / elapsed;
}

// 测试读写混合带宽 (memcpy)
double test_copy_bandwidth(char *src, char *dst, size_t size) {
    double start = get_time_seconds();
    
    for (int i = 0; i < ITERATIONS; i++) {
        memcpy(dst, src, size);
    }
    
    double end = get_time_seconds();
    double elapsed = end - start;
    // memcpy包含读和写，所以是2倍数据量
    double total_gb = (size * ITERATIONS * 2) / (1024.0 * 1024.0 * 1024.0);
    
    return total_gb / elapsed;
}

void dram_bandwidth_bm(void) {
    printf("\n========================================\n");
    printf("   DRAM Sustained Bandwidth Test\n");
    printf("========================================\n");
    printf("Buffer Size: %lu GB\n", BUFFER_SIZE / (1024 * 1024 * 1024));
    printf("Iterations: %d\n", ITERATIONS);
    printf("========================================\n\n");
    
    // 分配内存
    printf("Allocating memory...\n");
    char *buffer1 = (char*)malloc(BUFFER_SIZE);
    char *buffer2 = (char*)malloc(BUFFER_SIZE);
    
    if (!buffer1 || !buffer2) {
        printf("Error: Failed to allocate memory!\n");
        return 1;
    }
    
    // 初始化
    memset(buffer1, 0xAA, BUFFER_SIZE);
    memset(buffer2, 0x55, BUFFER_SIZE);
    
    printf("Running tests...\n\n");
    
    // 测试写带宽
    double write_bw = test_write_bandwidth(buffer1, BUFFER_SIZE);
    printf("Write Bandwidth:  %.2f GB/s\n", write_bw);
    
    // 测试读带宽
    double read_bw = test_read_bandwidth(buffer1, BUFFER_SIZE);
    printf("Read Bandwidth:   %.2f GB/s\n", read_bw);
    
    // 测试拷贝带宽
    double copy_bw = test_copy_bandwidth(buffer1, buffer2, BUFFER_SIZE);
    printf("Copy Bandwidth:   %.2f GB/s\n", copy_bw);
    
    printf("\n========================================\n");
    printf("Test completed!\n");
    printf("========================================\n");
    
    free(buffer1);
    free(buffer2);
    
}

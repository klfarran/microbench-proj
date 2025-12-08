#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <x86intrin.h>

#define REPEAT 10000000
#define CACHE_LINE_SIZE 64
#define CPUID() asm volatile ("CPUID" : : : "%rax", "%rbx", "%rcx", "%rdx", "memory");

// Node for pointer chasing
typedef struct Node {
    struct Node* next;
    char padding[CACHE_LINE_SIZE - sizeof(struct Node*)];
} __attribute__((aligned(64))) Node;


// Create pointer chasing chain
Node* create_pointer_chain(size_t size) {
    size_t num_nodes = size / sizeof(Node);
    Node* nodes = (Node*)aligned_alloc(CACHE_LINE_SIZE, size);
    
    if (!nodes) {
        printf("Memory allocation failed!\n");
        return NULL;
    }
    
    // Initialize memory
    memset(nodes, 0, size);
    
    // Create random access pattern
    size_t* indices = (size_t*)malloc(num_nodes * sizeof(size_t));
    for (size_t i = 0; i < num_nodes; i++) {
        indices[i] = i;
    }
    
    // Fisher-Yates shuffle
    for (size_t i = num_nodes - 1; i > 0; i--) {
        size_t j = rand() % (i + 1);
        size_t temp = indices[i];
        indices[i] = indices[j];
        indices[j] = temp;
    }
    
    // Build pointer chain
    for (size_t i = 0; i < num_nodes - 1; i++) {
        nodes[indices[i]].next = &nodes[indices[i + 1]];
    }
    nodes[indices[num_nodes - 1]].next = &nodes[indices[0]];
    
    free(indices);
    return nodes;
}

// Measure data access latency (L1D/L2/L3)
double measure_data_latency(Node* start, int iterations) {
    Node* p = start;
    uint64_t start_time, end_time;
	unsigned int aux;
    
    // Warmup
    for (int i = 0; i < 1000; i++) {
        p = p->next;
    }
    
    start_time = __rdtscp(&aux);
    
    for (int i = 0; i < iterations; i++) {
        p = p->next;
        p = p->next;
        p = p->next;
        p = p->next;
        p = p->next;
        p = p->next;
        p = p->next;
        p = p->next;
    }
    
    end_time = __rdtscp(&aux);
    
    // Prevent compiler optimization
    if (p == NULL) printf("");
    
    return (double)(end_time - start_time) / (iterations * 8);
}

// Generate jump chain instructions for L1I testing
void generate_jump_chain(char* buffer, size_t size) {
    // Jump instruction size (5 bytes: JMP)
    size_t jump_size = 5;
    size_t num_jumps = size / CACHE_LINE_SIZE;
    
    if (num_jumps == 0) num_jumps = 1;
    
    // Fill with NOPs
    memset(buffer, 0x90, size);
    
    // Create random jump chain
    size_t* indices = (size_t*)malloc(num_jumps * sizeof(size_t));
    for (size_t i = 0; i < num_jumps; i++) {
        indices[i] = i;
    }
    
    // Fisher-Yates shuffle
    for (size_t i = num_jumps - 1; i > 0; i--) {
        size_t j = rand() % (i + 1);
        size_t temp = indices[i];
        indices[i] = indices[j];
        indices[j] = temp;
    }
    
    // Generate jump chain
    for (size_t i = 0; i < num_jumps - 1; i++) {
        size_t from = indices[i] * CACHE_LINE_SIZE;
        size_t to = indices[i + 1] * CACHE_LINE_SIZE;
        int32_t offset = (int32_t)(to - from - jump_size);
        
        // JMP rel32 instruction: E9 [4-byte offset]
        buffer[from] = 0xE9;
        *(int32_t*)(buffer + from + 1) = offset;

	//new additions:
	size_t pos = from + jump_size;
	while (pos + 1 < from + CACHE_LINE_SIZE)
	    buffer[pos++] = 0x90;
    }
    
    // Last jump returns, then RET
    size_t last = indices[num_jumps - 1] * CACHE_LINE_SIZE;
    /*size_t first = indices[0] * CACHE_LINE_SIZE;
    int32_t offset = (int32_t)(first - last - jump_size);
    buffer[last] = 0xE9;
    *(int32_t*)(buffer + last + 1) = offset; */
    buffer[last] = 0xC3; // RET
    
    free(indices);
}

// Measure instruction cache latency (L1I) - using jump chain
double measure_instruction_latency(size_t code_size) {
    // Align to page size
    size_t page_size = sysconf(_SC_PAGESIZE);
    size_t aligned_size = ((code_size + page_size - 1) / page_size) * page_size;
    
    char* code = (char*)mmap(NULL, aligned_size, 
                             PROT_READ | PROT_WRITE | PROT_EXEC,
                             MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    
    if (code == MAP_FAILED) {
        return -1;
    }

   size_t lines = code_size / CACHE_LINE_SIZE;
   if (lines ==0) lines = 1;
    
    generate_jump_chain(code, code_size);
    
    void (*func)() = (void(*)())code;
    uint64_t start_time, end_time;
    int iterations = 1000000;
	unsigned int aux;
    
    // Warmup - load code into cache
    for (int i = 0; i < 1000; i++) {
        func();
    }
    
    // Measure
    Node* p = (Node*)code;
    start_time = __rdtscp(&aux);
    CPUID()
   
    for(int i = 0; i < iterations; i++) {
      //simulate each jump in the chain manually for each cache liNe
	for(size_t line = 0; line < lines; line++) {
      	   //fetch the instruction at this line to force I-cache access
   	   __asm__ volatile("" : : "r"(p) : "memory");
	   p = (Node*)((uintptr_t)p + CACHE_LINE_SIZE);
	   if ((uintptr_t)p >= (uintptr_t)code + code_size) {
		p = (Node*)code;
	   }
	}
    }

    CPUID()
    end_time = __rdtscp(&aux);
    
    munmap(code, aligned_size);
    
    // Return average cycles per jump
    size_t num_jumps = code_size / CACHE_LINE_SIZE;
    if (num_jumps == 0) num_jumps = 1;
    
    return (double)(end_time - start_time) / (iterations * num_jumps);
}

// Test L1D, L2, L3 cache
void test_data_cache() {
    printf("\n=== Data Cache Latency Test (L1D/L2/L3) ===\n");
    printf("%-15s %-15s %-15s\n", "Working Set", "Latency(cycles)", "Cache Level");
    printf("-----------------------------------------------\n");
    
    size_t sizes[] = {
        4 * 1024,        // 4 KB - L1D
        8 * 1024,        // 8 KB
        16 * 1024,       // 16 KB
        32 * 1024,       // 32 KB - L1D boundary
        64 * 1024,       // 64 KB
        128 * 1024,      // 128 KB - L2
        256 * 1024,      // 256 KB
        512 * 1024,      // 512 KB
        1 * 1024 * 1024, // 1 MB
        2 * 1024 * 1024, // 2 MB - L2/L3 boundary
        4 * 1024 * 1024, // 4 MB
        8 * 1024 * 1024, // 8 MB - L3
        16 * 1024 * 1024 // 16 MB
    };
    
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    double prev_latency = 0;
    
    for (int i = 0; i < num_sizes; i++) {
        Node* chain = create_pointer_chain(sizes[i]);
        if (!chain) continue;
        
        double latency = measure_data_latency(chain, REPEAT);
        
        // Determine cache level
        const char* cache_level = "";
        if (latency < 8) {
            cache_level = "L1D";
        } else if (latency < 20) {
            cache_level = "L2";
        } else if (latency < 60) {
            cache_level = "L3";
        } else {
            cache_level = "Memory";
        }
        
        // Format output
        if (sizes[i] < 1024 * 1024) {
            printf("%-13luKB %-15.2f %-15s", sizes[i] / 1024, latency, cache_level);
        } else {
            printf("%-13luMB %-15.2f %-15s", sizes[i] / (1024 * 1024), latency, cache_level);
        }
        
        // Show latency spike
        if (i > 0 && latency > prev_latency * 1.5) {
            printf(" ^^ Cache level transition");
        }
        printf("\n");
        
        prev_latency = latency;
        free(chain);
    }
}

// Test L1I cache
void test_instruction_cache() {
    printf("\n=== Instruction Cache Latency Test (L1I) ===\n");
    printf("%-15s %-15s %-15s\n", "Code Size", "Latency(cycles)", "Cache Level");
    printf("-----------------------------------------------\n");
    
    size_t sizes[] = {
        2 * 1024,   // 2 KB
        4 * 1024,   // 4 KB
        8 * 1024,   // 8 KB
        16 * 1024,  // 16 KB
        24 * 1024,  // 24 KB
        32 * 1024,  // 32 KB - L1I boundary
        48 * 1024,  // 48 KB
        64 * 1024,  // 64 KB - Beyond L1I
        128 * 1024, // 128 KB
        256 * 1024  // 256 KB
    };
    
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    double prev_latency = 0;
    
    for (int i = 0; i < num_sizes; i++) {
        double latency = measure_instruction_latency(sizes[i]);
        
        if (latency < 0) {
            printf("%-13luKB Test failed\n", sizes[i] / 1024);
            continue;
        }
        
        const char* cache_level = "";
        if (latency < 5) {
            cache_level = "L1I";
        } else if (latency < 15) {
            cache_level = "L2";
        } else if (latency < 50) {
            cache_level = "L3";
        }
        
        printf("%-13luKB %-15.2f %-15s", 
               sizes[i] / 1024, latency, cache_level);
        
        // Show cache level transition
        if (i > 0 && latency > prev_latency * 1.5) {
            printf(" ^^ Cache level transition");
        }
        printf("\n");
        
        prev_latency = latency;
    }
}

void cache_latency_bm(void) {
    srand(time(NULL));
    
    printf("=== CPU Cache Latency Benchmark ===\n");
    printf("Measuring L1I, L1D, L2, L3 cache access latency\n");
    
    test_instruction_cache();
    test_data_cache();
    
    printf("\n=== Summary ===\n");
    printf("L1I (Instruction): Typically 32 KB, < 5 cycles\n");
    printf("L1D (Data):        Typically 32-64 KB, 4-8 cycles (pointer chasing)\n");
    printf("L2:                Typically 256 KB - 1 MB, 10-20 cycles\n");
    printf("L3:                Typically 4-32 MB, 20-60 cycles\n");
    //printf("\nNote: L1D latency is higher due to pointer chasing overhead\n");
    
}

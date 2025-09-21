// Kelly Farran and Mohan Zhao
// Context Switch Overhead Microbenchmark
// CS 6354 Microbenchmarking Project

#include <stdio.h>
#include <unistd.h>     // for getpid(), pipe(), read(), write()
#include <pthread.h>    // for pthreads
#include <sched.h>      // for sched_setaffinity()
#include <x86intrin.h>  // for __rdtsc()


#ifdef _WIN32
#include <io.h>
#include <fcntl.h>   // for O_BINARY
#define pipe(fds) _pipe(fds, 4096, O_BINARY)
#endif


#define NUM_ITERATIONS 10000

// ------------------ Part 1: System call overhead ------------------
unsigned long long measure_syscall_overhead() {
    unsigned long long start, end, total = 0, ETbase = 0;

    // First measure rdtsc overhead
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        start = __rdtsc();
        end = __rdtsc();
        ETbase += end - start;
    }
    ETbase /= NUM_ITERATIONS;

    // Now measure getpid() syscall overhead
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        __asm__("CPUID"); // drain pipeline
        start = __rdtsc();
        getpid();         // get the process ID,simple system call (user ↔ kernel)
        __asm__("CPUID");
        end = __rdtsc();
        total += (end - start) - ETbase;
    }
    return total / NUM_ITERATIONS;
}

// ------------------ Part 2: Thread context switch ------------------
unsigned long long measure_syscall_pipe_overhead() {

    
    // calculate the time for a single thread (read...write...)
    int my_pipe[2];
    pipe(my_pipe);
    
    char buf = 'x';
    char rbuf;
    unsigned long long start, end, total = 0;
    write(my_pipe[1], &buf, 1); //make sure the single pipe is not empty for read
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        __asm__("CPUID");
        start = __rdtsc();
        
        write(my_pipe[1], &buf, 1);
        read(my_pipe[0], &rbuf, 1);
        
        __asm__("CPUID");
        end = __rdtsc();
        total += (end - start);
    }
    read(my_pipe[0], &rbuf, 1); // read the last data the clear
    close(my_pipe[0]);
    close(my_pipe[1]);
    return total / NUM_ITERATIONS; // single thread, average time of write+read
}

int pipefd1[2], pipefd2[2];  // two pipes for ping-pong
pthread_barrier_t barrier;   // to synchronize start


void* ping_thread(void* arg) {
    char buf = 'x';
    char rbuf;
    pthread_barrier_wait(&barrier);

    for (int i = 0; i < NUM_ITERATIONS; i++) {
        // Write a token to thread B
        read(pipefd1[0], &buf, 1);
        // Wait for reply from thread B
        write(pipefd2[1], &rbuf, 1);
    }
    return NULL;
}

unsigned long long measure_thread_switch_overhead() {
    pthread_t t;
    char buf = 'x';
    char rbuf;
    unsigned long long start, end, total = 0,ETbase = 0;

    // First measure rdtsc overhead
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        start = __rdtsc();
        end = __rdtsc();
        ETbase += end - start;
    }

    ETbase /= NUM_ITERATIONS;;
        // single thread overhead (without switch)
    unsigned long long base_overhead = measure_syscall_pipe_overhead();

    // Prepare pipes
    pipe(pipefd1);
    pipe(pipefd2);

    // Set up barrier for sync between two threads
    pthread_barrier_init(&barrier, NULL, 2);

    // Launch thread B (pong)
    pthread_create(&t, NULL, ping_thread, NULL);

    // Synchronize start
    pthread_barrier_wait(&barrier);

    // Measure one full ping-pong = 2 context switches
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        __asm__("CPUID");
        start = __rdtsc();

        // Thread A writes to B
        write(pipefd1[1], &buf, 1);
        // Thread A waits for reply
        read(pipefd2[0], &rbuf, 1);

        __asm__("CPUID");
        end = __rdtsc();
        total += (end - start) - 2 * base_overhead+ETbase; 
        //for each iteration, thread A and B both have read&write 
        //and together we only have one time measurement cost (ETbase). 
        //In base_overhead, we count the time measurement cost in it.
        //So that base_overhead = 1 read&write+ 1 ETbase;
        //since we need 2 * read&write + 1 * ETbase.
        //= 2 * (base_overhead-ETbase) + ETbase = 2 * base_overhead - ETbase
        // substract it,
        // total += (end - start) - (2 * base_overhead-ETbase).

        //however, in this function we repeat the calculation of ETbase again,
        //which appears first in measure_syscall_overhead()
        // we could make it a global variable later to make the code tidy.

    }

    pthread_join(t, NULL);
    

    return (total/NUM_ITERATIONS) / 2;
    // Each iteration has 2 switches, so divide by 2

}


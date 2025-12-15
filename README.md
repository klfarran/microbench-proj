# CPU/ Memory Performance Microbenchmark Suite
This project contains a set of 11 microbenchmarks which assess various aspects of a CPU / memory system. The microbenchmarks included are as follows: 
- **Function Call Overhead**
  - Measure the cost (in cycles) of invoking a function with an empty body, evaluated as a function of the number and type of arguments
  - Note- to change the type of argument passed, change the VAR_TYPE macro found in include/config.h

- **Context Switch Overhead**
  Quantify context switch latency using two approaches:  
  - (a) system calls transitioning between user and kernel space  
  - (b) switches between two pinned threads (e.g., pipe ping-pong)
    
- Instruction Fetch Throughput
  - Determine max instructions fetched per cycle using tight loops
  - Also tests the effects of instruction length and alignment on max instructions fetched per cycle 
    
- Effective Instruction Throughput 
  -  Measure min, max, and median instruction throughput (retired instructions) for varying levels of ILP 
    
- Load/Store Service Rate 
  -  Number of loads and stores served per cycle under full bandwidth
    
- Branch Misprediction Penalty
  - Pipeline flush latency due to mispredictions
    
- Integer Execution Unit Bandwidth
  - Max INT ALU operations executed per cycle (split by simple ALU ops, multiply, and divide)

- Cache Latencies (L1I/L1D/L2/L3)
  - Per-level access time via pointer chasing or array stride
    
- Cache Bandwidth (L1I/L1D/L2/L3)
  - Sustained read/write/copy throughput
    
- Main Memory (DRAM) Latency
  - Load latency beyond the last-level cache

- Main Memory (DRAM) Bandwidth
  - Sustained read/write/copy throughput

- SMT Contention & Symbiosis
  - Impact of co-scheduled threads (competing vs. complementary mixes)


## Build
```bash
make -clean
```

## Run
```bash
scripts/run_all.sh
```

## Notes
- Please configure several parameters in the file include/config.h to match your system specifications
  - VAR_TYPE: The type of argument to be passed to the function call overhead microbenchmark
  - cpu_freq_ghz: The clock frequency of your CPU
  - NUM_ITERATIONS: The number of iterations for the microbenchmark test loops  

# CPU Performance Microbenchmark Suite

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

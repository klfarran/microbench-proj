# CPU Performance Microbenchmarks (Starter Kit)

## Build
```bash
make -clean
```

## Run
```bash
scripts/run_all.sh
```

## Notes
- The type of argument to be passed to the function call overhead microbenchmark is specified by the VAR_TYPE macro in the file include/config.h. To run the microbenchmark with arguments of a different data type, all that is needed is to change the VAR_TYPE macro to be the data type you would like to pass.
- Similarly, the number of iterations for the microbenchmark test loops are specified by the NUM_ITERATIONS macro in include/config.h   

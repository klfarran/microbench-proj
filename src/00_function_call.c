// Kelly Farran and Mohan Zhao
// Function Call Overhead Microbenchmark
// CS 6354 Microbenchmarking Project

#include "00_function_call.h"
#include <stdio.h>
#include <x86intrin.h> //for access to __rdtsc(), assumes a GCC compiler and an x86 instruction set

#include "config.h" //includes macros for NUM_ITERATIONS and VAR_TYPE

	//one argument function
	void Func1a(VAR_TYPE a1) {
	
	}
	
	//two arguments
	void Func2a(VAR_TYPE a1, VAR_TYPE a2) {

	}	
	
	//four arguments
	void Func4a(VAR_TYPE a1, VAR_TYPE a2, VAR_TYPE a3, VAR_TYPE a4) {
		
	}
	
	//six arguments
	void Func6a(VAR_TYPE a1, VAR_TYPE a2, VAR_TYPE a3, VAR_TYPE a4, VAR_TYPE a5, VAR_TYPE a6) {
		
	}
	
	//eight arguments
	void Func8a(VAR_TYPE a1, VAR_TYPE a2, VAR_TYPE a3, VAR_TYPE a4, VAR_TYPE a5, VAR_TYPE a6, VAR_TYPE a7, VAR_TYPE a8) {
		
	}
	
	//fifteen arguments
	void Func15a(VAR_TYPE a1, VAR_TYPE a2, VAR_TYPE a3, VAR_TYPE a4, VAR_TYPE a5, VAR_TYPE a6, VAR_TYPE a7, VAR_TYPE a8, VAR_TYPE a9, VAR_TYPE a10, VAR_TYPE a11, VAR_TYPE a12, VAR_TYPE a13, VAR_TYPE a14, VAR_TYPE a15) {
		
	}
	

	void function_overhead_bm(void) {			
							
		//measure overhead of rdtsc instruction
		unsigned long long start = 0, end = 0, ETbase = 0;
		
		for(int i = 0; i < NUM_ITERATIONS; i++) {
			start = __rdtsc();
			end = __rdtsc();
			ETbase += end - start;
		}
		
		ETbase /= NUM_ITERATIONS;
		
		
		//initialize all function arguments 
		VAR_TYPE a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15; 	
		
		unsigned long long funcStart = 0, funcEnd = 0;
		unsigned long long Func1aTotalET = 0, Func2aTotalET = 0, Func4aTotalET = 0, Func6aTotalET = 0, Func8aTotalET = 0, Func15aTotalET = 0;
		
		
		for(int i = 0; i < NUM_ITERATIONS; i++) {

			//measure cycles for function call with one argument
			__asm__("CPUID"); //embedded x86 assembly- drain pipeline
			funcStart = __rdtsc(); 
			Func1a(a1);	
			__asm__("CPUID"); //drain pipeline
			funcEnd = __rdtsc();
			Func1aTotalET += (funcEnd - funcStart) - ETbase;
			
			//measure cycles for function call with two arguments
			__asm__("CPUID"); 
			funcStart = __rdtsc(); 
			Func2a(a1, a2);	
			__asm__("CPUID"); 
			funcEnd = __rdtsc();
			Func2aTotalET += (funcEnd - funcStart) - ETbase;
			
			//measure cycles for function call with four arguments
			__asm__("CPUID"); 
			funcStart = __rdtsc(); 
			Func4a(a1, a2, a3, a4);	
			__asm__("CPUID"); 
			funcEnd = __rdtsc();
			Func4aTotalET += (funcEnd - funcStart) - ETbase;
			
			//measure cycles for function call with six arguments
			__asm__("CPUID"); 
			funcStart = __rdtsc(); 
			Func6a(a1, a2, a3, a4, a5, a6);	
			__asm__("CPUID"); 
			funcEnd = __rdtsc();
			Func6aTotalET += (funcEnd - funcStart) - ETbase;
			
			//measure cycles for function call with eight arguments
			__asm__("CPUID"); 
			funcStart = __rdtsc(); 
			Func8a(a1, a2, a3, a4, a5, a6, a7, a8);	
			__asm__("CPUID"); 
			funcEnd = __rdtsc();
			Func8aTotalET += (funcEnd - funcStart) - ETbase;
			
			//measure cycles for function call with fifteen arguments
			__asm__("CPUID"); 
			funcStart = __rdtsc(); 
			Func15a(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);	
			__asm__("CPUID"); 
			funcEnd = __rdtsc();
			Func15aTotalET += (funcEnd - funcStart) - ETbase;
		}
		
		printf("\nAverage cycles for function with one argument: %llu\nAverage cycles for function with two arguments: %llu\nAverage cycles for function with four arguments: %llu", 
			Func1aTotalET/NUM_ITERATIONS, Func2aTotalET/NUM_ITERATIONS, Func4aTotalET/NUM_ITERATIONS);
		printf("\nAverage cycles for function with six arguments: %llu\nAverage cycles for function with eight arguments: %llu\nAverage cycles for function with fifteen arguments: %llu\n\n", 
			Func6aTotalET/NUM_ITERATIONS, Func8aTotalET/NUM_ITERATIONS, Func15aTotalET/NUM_ITERATIONS);
				
		
	}
	
	

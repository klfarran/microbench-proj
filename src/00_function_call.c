#include "00_function_call.h"
#include <stdio.h>
#include <x86intrin.h> //for access to __rdtsc(), assumes a GCC compiler and an x86 instruction set

#include "config.h" //includes macros for NUM_ITERATIONS and VAR_TYPE

	//zero argument function
	void Func0(void) {
	
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
	
	//ten arguments
	void Func10a(VAR_TYPE a1, VAR_TYPE a2, VAR_TYPE a3, VAR_TYPE a4, VAR_TYPE a5, VAR_TYPE a6, VAR_TYPE a7, VAR_TYPE a8, VAR_TYPE a9, VAR_TYPE a10) {
		
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
		//QUESTION- do these args have to be initialized? 
		VAR_TYPE a1, a2, a3, a4, a5, a6, a7, a8, a9, a10; 	
			//void(a1);
		
		unsigned long long funcStart = 0, funcEnd = 0;
		unsigned long long emptyFuncTotalET = 0, Func2aTotalET = 0, Func4aTotalET = 0, Func6aTotalET = 0, Func8aTotalET = 0, Func10aTotalET = 0;
		
		
		for(int i = 0; i < NUM_ITERATIONS; i++) {
			//measure cycles for function call with zero arguments
			__asm__("CPUID"); //embedded x86 assembly- drain pipeline
			funcStart = __rdtsc(); 
			Func0();	
			__asm__("CPUID"); //drain pipeline
			funcEnd = __rdtsc();
			emptyFuncTotalET += (funcEnd - funcStart) - ETbase;
			
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
			
			//measure cycles for function call with ten arguments
			__asm__("CPUID"); 
			funcStart = __rdtsc(); 
			Func10a(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);	
			__asm__("CPUID"); 
			funcEnd = __rdtsc();
			Func10aTotalET += (funcEnd - funcStart) - ETbase;
		}
		
		printf("\nNum cycles for function with zero arguments: %llu\nNum cycles for function with two arguments: %llu\nNum cycles for function with four arguments: %llu", 
			emptyFuncTotalET/NUM_ITERATIONS, Func2aTotalET/NUM_ITERATIONS, Func4aTotalET/NUM_ITERATIONS);
		printf("\nNum cycles for function with six arguments: %llu\nNum cycles for function with eight arguments: %llu\nNum cycles for function with ten arguments: %llu\n", 
			Func6aTotalET/NUM_ITERATIONS, Func8aTotalET/NUM_ITERATIONS, Func10aTotalET/NUM_ITERATIONS);
				
		
	}
	
	
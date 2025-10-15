#include "00_function_call.h"
#include <stdio.h>
#include <x86intrin.h> //for access to __rdtsc(), assumes a GCC compiler and an x86 instruction set
#include "config.h" 

	//zero argument function
	void Func0() {

	}

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
	
	//ten arguments
	void Func15a(VAR_TYPE a1, VAR_TYPE a2, VAR_TYPE a3, VAR_TYPE a4, VAR_TYPE a5, VAR_TYPE a6, VAR_TYPE a7, VAR_TYPE a8, VAR_TYPE a9, VAR_TYPE a10, VAR_TYPE a11, VAR_TYPE a12, VAR_TYPE a13, VAR_TYPE a14, VAR_TYPE a15) {
		
	}
	

	void function_overhead_bm(void) {			
							
		//measure overhead of rdtsc instruction
		unsigned long long start = 0, end = 0, ETbase = 0;
		unsigned int aux;

		for(int i = 0; i < NUM_ITERATIONS; i++) {
			__asm__ __volatile__("cpuid"::: "%rax", "%rbx", "%rcx", "%rdx", "memory");
			start = __rdtscp(&aux);
			end = __rdtscp(&aux);
			__asm__ __volatile__("cpuid"::: "%rax", "%rbx", "%rcx", "%rdx", "memory");
			ETbase += end - start;
		}
		
		ETbase /= NUM_ITERATIONS;		
		
		//initialize all function arguments 
		VAR_TYPE a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15; 	
		
		unsigned long long funcStart = 0, funcEnd = 0;
		unsigned long long Func0TotalET = 0, Func1aTotalET = 0, Func2aTotalET = 0, Func4aTotalET = 0, Func6aTotalET = 0, Func8aTotalET = 0, Func15aTotalET = 0;
				
		for(int i = 0; i < NUM_ITERATIONS; i++) {
			//measure cycles for function call with zero arguments
			__asm__ __volatile__("cpuid"::: "%rax", "%rbx", "%rcx", "%rdx", "memory");
			funcStart = __rdtscp(&aux); 
			Func0();
			funcEnd = __rdtscp(&aux);
			__asm__ __volatile__("cpuid"::: "%rax", "%rbx", "%rcx", "%rdx", "memory");
			Func0TotalET += (funcEnd - funcStart) - ETbase;
				
			// one argument
			__asm__ __volatile__("cpuid"::: "%rax", "%rbx", "%rcx", "%rdx", "memory");
			funcStart = __rdtscp(&aux); 
			Func1a(a1);	
			funcEnd = __rdtscp(&aux);
			__asm__ __volatile__("cpuid"::: "%rax", "%rbx", "%rcx", "%rdx", "memory");
			Func1aTotalET += (funcEnd - funcStart) - ETbase;
			
			// two arguments
			__asm__ __volatile__("cpuid"::: "%rax", "%rbx", "%rcx", "%rdx", "memory");
			funcStart = __rdtscp(&aux); 
			Func2a(a1, a2);	
			funcEnd = __rdtscp(&aux);
			__asm__ __volatile__("cpuid"::: "%rax", "%rbx", "%rcx", "%rdx", "memory");
			Func2aTotalET += (funcEnd - funcStart) - ETbase;
			
			// four arguments
			__asm__ __volatile__("cpuid"::: "%rax", "%rbx", "%rcx", "%rdx", "memory");
			funcStart = __rdtscp(&aux); 
			Func4a(a1, a2, a3, a4);	
			funcEnd = __rdtscp(&aux);
			__asm__ __volatile__("cpuid"::: "%rax", "%rbx", "%rcx", "%rdx", "memory");
			Func4aTotalET += (funcEnd - funcStart) - ETbase;
			
			// six arguments 
			__asm__ __volatile__("cpuid"::: "%rax", "%rbx", "%rcx", "%rdx", "memory");
			funcStart = __rdtscp(&aux); 
			Func6a(a1, a2, a3, a4, a5, a6);	
			funcEnd = __rdtscp(&aux);
			__asm__ __volatile__("cpuid"::: "%rax", "%rbx", "%rcx", "%rdx", "memory");
			Func6aTotalET += (funcEnd - funcStart) - ETbase;
			
			// eight arguments
			__asm__ __volatile__("cpuid"::: "%rax", "%rbx", "%rcx", "%rdx", "memory");
			funcStart = __rdtscp(&aux); 
			Func8a(a1, a2, a3, a4, a5, a6, a7, a8);	
			funcEnd = __rdtscp(&aux);
			__asm__ __volatile__("cpuid"::: "%rax", "%rbx", "%rcx", "%rdx", "memory");
			Func8aTotalET += (funcEnd - funcStart) - ETbase;
			
			// ten arguments 
			__asm__ __volatile__("cpuid"::: "%rax", "%rbx", "%rcx", "%rdx", "memory");
			funcStart = __rdtscp(&aux); 
			Func15a(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);	
			funcEnd = __rdtscp(&aux);
			__asm__ __volatile__("cpuid"::: "%rax", "%rbx", "%rcx", "%rdx", "memory");
			Func15aTotalET += (funcEnd - funcStart) - ETbase;
		}
		printf("\nFunction with no arguments: %llu cycles", Func0TotalET/NUM_ITERATIONS);
		printf("\nFunction with one argument: %llu cycles\nFunction with two arguments: %llu cycles\nFunction with four arguments: %llu cycles", 
			Func1aTotalET/NUM_ITERATIONS, Func2aTotalET/NUM_ITERATIONS, Func4aTotalET/NUM_ITERATIONS);
		printf("\nFunction with six arguments: %llu cycles\nFunction with eight arguments: %llu cycles\nFunction with fifteen arguments: %llu cycles\n\n", 
			Func6aTotalET/NUM_ITERATIONS, Func8aTotalET/NUM_ITERATIONS, Func15aTotalET/NUM_ITERATIONS);
				
		
	}
	
	

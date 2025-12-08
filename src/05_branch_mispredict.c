// Kelly Farran and Mohan Zhao
// Branch Misprediction Microbenchmark
// CS 6354 Microbenchmarking Project

#include "05_branch_mispredict.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <inttypes.h>
#include <x86intrin.h>

#define ITER (1UL)
#define TRAIN_COUNT (1000UL)
#define RUNS (100000UL)
#define CPUID asm volatile ("CPUID" : : : "%rax", "%rbx", "%rcx", "%rdx", "memory");
#define NOP asm volatile ("");


	static inline uint64_t rdtsc_overhead(void) {
		uint64_t s,e,sum=0;
		unsigned int aux;
		for (int i=0;i<10000;i++){
			CPUID
			uint64_t s = __rdtscp(&aux);
			uint64_t e = __rdtscp(&aux);
			CPUID
			sum += (e - s);
		}
		return sum / 10000;
	}

	//This function, f1, is trained to be always taken 
	void f1_train(uint64_t count) {
		int condition = 1; //always true
		
		for(uint64_t i = 0; i < count; i++) {
			if(condition) {
				NOP //taken 
			} else {
				NOP //not taken 
			}				
		}	
	}
	
	//This function, f2, measures the time when the branch is predicted correctly (taken)
	uint64_t f2_taken(uint64_t count) {
		uint64_t start, end;
		unsigned int aux;
		int condition = 1; //always true- same as training in f1 
		
		CPUID
		start = __rdtscp(&aux); 
		
		for(uint64_t i = 0; i < count; i++) {
			if(condition) {
				NOP //taken 
			} else {
				NOP //not taken 
			}
		}
		
		end = __rdtscp(&aux);
		CPUID
		
		return end-start;
	}
	
	//This function, f3, measures the time when the branch is predicted incorrectly
	// (predicted taken, actual not taken)
	uint64_t f3_not_taken(uint64_t count) {
		uint64_t start, end;
		unsigned int aux;
		int condition = 0; //always false- opposite as training in f1 
		
		CPUID
		start = __rdtscp(&aux);
		
		for(uint64_t i = 0; i < count; i++) {
			if(condition) {
				NOP  //taken 
			} else {
				NOP  //not taken 
			}
		}
		
		end = __rdtscp(&aux);
		CPUID
		
		return end-start; 
		
	}
	
	
	//calculate and print out measurements
	void branch_mispredict_bm(void) {	
		/*
		uint64_t cycles_taken, cycles_not_taken;
		double cpb_taken, cpb_not_taken;
		double mispredict_penalty;
		
		//train f1 to expect the branch to be taken 
		f1_train(TRAIN_COUNT);
		
		//measure predictable f2 (branch taken, predicted correctly) 
		cycles_taken = f2_taken(ITER);
		
		//measure mispredicted f3 (branch is not taken, incorrectly predicted)
		cycles_not_taken = f3_not_taken(ITER);
		
		cpb_taken = (double)cycles_taken / ITER;
		cpb_not_taken = (double)cycles_not_taken / ITER; */
		
		uint64_t overhead = rdtsc_overhead();
		uint64_t total_cycles_taken = 0;
		uint64_t total_cycles_not_taken = 0;
		
		for(uint64_t i = 0; i < RUNS; i++) {
			//retrain branch pred every run to ensure we start with predicted-taken state
			f1_train(TRAIN_COUNT);
			
			//measure correctly predicted 
			total_cycles_taken += f2_taken(ITER);
			
			//retrain again to re-establish predicted-taken state
			f1_train(TRAIN_COUNT);
			
			//measure incorrectly predicted
			total_cycles_not_taken += f3_not_taken(ITER);
		}
		
		uint64_t total_branches = ITER * RUNS;
		
		if(total_cycles_taken - (overhead*RUNS) > 0) {
			total_cycles_taken -= (overhead*RUNS);
		}
		
		if(total_cycles_not_taken - (overhead*RUNS) > 0) {
			total_cycles_not_taken -= (overhead*RUNS);
		}
		
		double cpb_taken = (double)total_cycles_taken / total_branches;
		double cpb_not_taken = (double)total_cycles_not_taken / total_branches;
		
		double mispredict_penalty = cpb_not_taken - cpb_taken;
		
		printf("\n=== Isolated Branch Misprediction Penalty Measurement ===\n\n");
		
		printf("Time(F2, Predictable): %.4f cycles/branch (Ideal)\n", cpb_taken);
		printf("Time(F3, Mispredicted): %.4f cycles/branch (Penalty + Ideal)\n\n", cpb_not_taken);
		printf("Misprediction Penalty (Time(F3) - Time(F2)): %.2f cycles\n", mispredict_penalty);
		
	  
		
		printf("\n--------------------------------------------\n\n");
	}

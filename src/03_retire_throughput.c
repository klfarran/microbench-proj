// Kelly Farran and Mohan Zhao
// Context Switch Overhead Microbenchmark
// CS 6354 Microbenchmarking Project

#include "03_retire_throughput.h"
#include "config.h"
#include <stdio.h>
#include <x86intrin.h> //for __rdtscp()

	void retire_throughput_bm(void) {	
		
		//initialize values
		unsigned int aux;
		unsigned long long start = 0, end = 0, AvgET = 0, ETbase = 0;
		int a = 0, b = 0, c = 0, d = 0, e = 0, f = 0, g = 0, h = 0, i = 0, j = 0;
		int arr[2] = {1};
		
		//calculate ETbase
		for(int i = 0; i < NUM_ITERATIONS; i++) {
			start = __rdtsc();
			end = __rdtsc();
			ETbase += end - start;
		}
		
		ETbase /= NUM_ITERATIONS;	
		
// ------------------ Measure instructions retired per cycle ------------------	
		
		//Get Max IPC -> Execute block of high ILP instructions
		for(int i = 0; i < NUM_ITERATIONS; i++) {
			start = __rdtscp(&aux);
			a++;
			b++;
			c++;
			d++;
			/*e++;
			f++;
			g++;
			h++;
			i++;
			j++;*/
			end = __rdtscp(&aux);
			AvgET += (end - start) - ETbase;
		}
		
			AvgET /= NUM_ITERATIONS;
			printf("Max IPC: %f\n", 4.0/AvgET);
			printf("AvgET: %llu\n\n", AvgET);
			
			
		//Get Med IPC -> Execute block of Med ILP instructions 
		AvgET = 0;
		for(int i = 0; i < NUM_ITERATIONS; i++) {
			start = __rdtscp(&aux);
			a++;
			b++;
			a++;
			b++;
			end = __rdtscp(&aux);
			AvgET += (end - start) - ETbase;
		}
		
			AvgET /= NUM_ITERATIONS;
			printf("Median IPC: %f\n", 2.0/AvgET);
			printf("AvgET: %llu\n\n", AvgET);
			
			
		//Get Min IPC -> Execute block of low ILP instructions
		AvgET = 0;
		for(int i = 0; i < NUM_ITERATIONS; i++) {
			start = __rdtscp(&aux);
			a++;
			a++;
			a++;
			a++;
			/*a = arr[0]; 
			arr[0] = a + 1;*/
			end = __rdtscp(&aux);
			AvgET += (end - start) - ETbase;
		}
		
			AvgET /= NUM_ITERATIONS;
			printf("Min IPC: %f\n", 2.0/AvgET);
			printf("AvgET: %llu\n\n", AvgET);
		
	}
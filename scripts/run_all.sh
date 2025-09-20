#!/usr/bin/env bash
# Code to automatically compile and run all benchmarks

	#Compile benchmarks
	make

	#Run benchmarks from harness.c
	./build/harness
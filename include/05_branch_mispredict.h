#ifndef BRANCH_MISPREDICT_H
#define BRANCH_MISPREDICT_H

#include <stdint.h>

	static inline uint64_t rdtsc_overhead(void);
	void f1_train(uint64_t count);
	uint64_t f2_taken(uint64_t count);
	uint64_t f3_not_taken(uint64_t count);
	void branch_mispredict_bm(void);

#endif

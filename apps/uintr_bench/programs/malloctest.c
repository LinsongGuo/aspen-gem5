#include "stdio.h"
#include "programs.h"

#include <x86intrin.h>



long long malloctest() {
	long long sum = 0;
	
	for (int i = 0; i < 1000000; ++i) {
		int *data = (int*) malloc(2048);
		sum += data[i & 512];
		free(data);
	}

	return sum;
}

// malloc(1024)
// w/ disable/enable:  0.018856960 s
// w/o:  0.014726656 s
// 2.065152e-09

// malloc(4096)
// w/ disable/enable:   0.063456512 s
// w/o:  0.060956928 s
// 1.249792000000003e-09
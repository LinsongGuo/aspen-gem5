#include "stdio.h"
#include "programs.h"

#include <x86intrin.h>

const int N = 5e4;
long long sum() {
	long long res = 0;
	int i, j;
	for (i = 0; i < N; ++i) {
		for (j = N - 1; j >= 0; --j) {
			res += i | j;
		}
	}
	// _clui();
	// printf("%lld\n", res);
	// _stui();
	return res;
}
